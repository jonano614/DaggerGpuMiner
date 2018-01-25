/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file Guards.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>

using Mutex = std::mutex;
using Guard = std::lock_guard<std::mutex>;
using UniqueGuard = std::unique_lock<std::mutex>;

template <class GuardType, class MutexType>
struct GenericGuardBool: GuardType
{
	GenericGuardBool(MutexType& _m): GuardType(_m) {}
	bool b = true;
};

template <class N>
class Notified
{
public:
	Notified() {}
	Notified(N const& _v): m_value(_v) {}
	Notified(Notified const&) = delete;
	Notified& operator=(N const& _v) { UniqueGuard l(m_mutex); m_value = _v; m_cv.notify_all(); return *this; }

	operator N() const { UniqueGuard l(m_mutex); return m_value; }

	void wait() const { N old; { UniqueGuard l(m_mutex); old = m_value; } waitNot(old); }
	void wait(N const& _v) const { UniqueGuard l(m_mutex); m_cv.wait(l, [&](){return m_value == _v;}); }
	void waitNot(N const& _v) const { UniqueGuard l(m_mutex); m_cv.wait(l, [&](){return m_value != _v;}); }
	template <class F> void wait(F const& _f) const { UniqueGuard l(m_mutex); m_cv.wait(l, _f); }

	template <class R, class P> void wait(std::chrono::duration<R, P> _d) const { N old; { UniqueGuard l(m_mutex); old = m_value; } waitNot(_d, old); }
	template <class R, class P> void wait(std::chrono::duration<R, P> _d, N const& _v) const { UniqueGuard l(m_mutex); m_cv.wait_for(l, _d, [&](){return m_value == _v;}); }
	template <class R, class P> void waitNot(std::chrono::duration<R, P> _d, N const& _v) const { UniqueGuard l(m_mutex); m_cv.wait_for(l, _d, [&](){return m_value != _v;}); }
	template <class R, class P, class F> void wait(std::chrono::duration<R, P> _d, F const& _f) const { UniqueGuard l(m_mutex); m_cv.wait_for(l, _d, _f); }

private:
	mutable Mutex m_mutex;
	mutable std::condition_variable m_cv;
	N m_value;
};

#define DEV_GUARDED(MUTEX) \
	for (GenericGuardBool<Guard, Mutex> __eth_l(MUTEX); __eth_l.b; __eth_l.b = false)
