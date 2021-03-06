/*
	Copyright 2014 Francesco "Franc[e]sco" Noferi (francesco149@gmail.com)

	This file is part of wxPloiter.

	wxPloiter is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	wxPloiter is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with wxPloiter. If not, see <http://www.gnu.org/licenses/>.
*/

#include "crypt.hpp"
#include "aes.hpp"
#include "utils.hpp"

// this is all refactored code from titanms, credits to whoever made this
// TODO: refactor the unreadable encryption crap

namespace maple
{
	namespace asmop = utils::asmop;

	crypt::crypt(word maple_version, byte *iv)
	{
		// the cypher key is repeated 4 times
		for (byte i = 0; i < 4; i++)
			std::copy(iv, iv + 4, this->iv + i * 4);

		this->maple_version = ((maple_version >> 8) & 0xFF) | 
			((maple_version << 8) & 0xFF00);
	}

	crypt::~crypt()
	{
		// empty
	}

	word crypt::getmapleversion()
	{
		return maple_version;
	}

	void crypt::encrypt(byte *buffer, signed_dword cb)
	{
		maplecrypt(buffer, cb);
		aes::get()->encrypt(buffer, iv, cb);
	}

	void crypt::decrypt(byte *buffer, signed_dword cb)
	{
		aes::get()->decrypt(buffer, iv, cb);
		mapledecrypt(buffer, cb);
	}

	bool crypt::check(byte *buffer)
	{
		// I have no idea what I'm doing but it works so don't touch this
		return ((((buffer[0] ^ iv[2]) & 0xFF) == ((maple_version >> 8) & 0xFF)) && 
			(((buffer[1] ^ iv[3]) & 0xFF) == (maple_version & 0xFF)));
	}

	void crypt::makeheader(byte *buffer, word cb)
	{
		// I have no idea what I'm doing
		// this encryption shit was reversed from the game itself
		word iiv = (iv[3]) & 0xFF;
		iiv |= (iv[2] << 8) & 0xFF00;

		iiv ^= maple_version;
		word mlength = ((cb << 8) & 0xFF00) | (cb >> 8);
		word xoredIv = iiv ^ mlength;

		buffer[0] = static_cast<byte>((iiv >> 8) & 0xFF);
		buffer[1] = static_cast<byte>(iiv & 0xFF);
		buffer[2] = static_cast<byte>((xoredIv >> 8) & 0xFF);
		buffer[3] = static_cast<byte>(xoredIv & 0xFF);
	}

	word crypt::length(byte *header)
	{
		return (static_cast<word>(header[0]) + static_cast<word>(header[1]) * 0x100) 
			 ^ (static_cast<word>(header[2]) + static_cast<word>(header[3]) * 0x100); 
	}

	void crypt::nextiv()
	{
		nextiv(iv);
	}

	void crypt::nextiv(byte *vector)
	{
		// I have no idea what I'm doing
		// this encryption shit was reversed from the game itself
		// credits to vana for the key shuffle code
		static const byte im12andwhatisthis[256] = { 
			0xEC, 0x3F, 0x77, 0xA4, 0x45, 0xD0, 0x71, 0xBF, 0xB7, 0x98, 0x20, 0xFC, 0x4B, 0xE9, 0xB3, 0xE1,
            0x5C, 0x22, 0xF7, 0x0C, 0x44, 0x1B, 0x81, 0xBD, 0x63, 0x8D, 0xD4, 0xC3, 0xF2, 0x10, 0x19, 0xE0,
            0xFB, 0xA1, 0x6E, 0x66, 0xEA, 0xAE, 0xD6, 0xCE, 0x06, 0x18, 0x4E, 0xEB, 0x78, 0x95, 0xDB, 0xBA,
            0xB6, 0x42, 0x7A, 0x2A, 0x83, 0x0B, 0x54, 0x67, 0x6D, 0xE8, 0x65, 0xE7, 0x2F, 0x07, 0xF3, 0xAA,
            0x27, 0x7B, 0x85, 0xB0, 0x26, 0xFD, 0x8B, 0xA9, 0xFA, 0xBE, 0xA8, 0xD7, 0xCB, 0xCC, 0x92, 0xDA,
            0xF9, 0x93, 0x60, 0x2D, 0xDD, 0xD2, 0xA2, 0x9B, 0x39, 0x5F, 0x82, 0x21, 0x4C, 0x69, 0xF8, 0x31,
            0x87, 0xEE, 0x8E, 0xAD, 0x8C, 0x6A, 0xBC, 0xB5, 0x6B, 0x59, 0x13, 0xF1, 0x04, 0x00, 0xF6, 0x5A,
            0x35, 0x79, 0x48, 0x8F, 0x15, 0xCD, 0x97, 0x57, 0x12, 0x3E, 0x37, 0xFF, 0x9D, 0x4F, 0x51, 0xF5,
            0xA3, 0x70, 0xBB, 0x14, 0x75, 0xC2, 0xB8, 0x72, 0xC0, 0xED, 0x7D, 0x68, 0xC9, 0x2E, 0x0D, 0x62,
            0x46, 0x17, 0x11, 0x4D, 0x6C, 0xC4, 0x7E, 0x53, 0xC1, 0x25, 0xC7, 0x9A, 0x1C, 0x88, 0x58, 0x2C,
            0x89, 0xDC, 0x02, 0x64, 0x40, 0x01, 0x5D, 0x38, 0xA5, 0xE2, 0xAF, 0x55, 0xD5, 0xEF, 0x1A, 0x7C,
            0xA7, 0x5B, 0xA6, 0x6F, 0x86, 0x9F, 0x73, 0xE6, 0x0A, 0xDE, 0x2B, 0x99, 0x4A, 0x47, 0x9C, 0xDF,
            0x09, 0x76, 0x9E, 0x30, 0x0E, 0xE4, 0xB2, 0x94, 0xA0, 0x3B, 0x34, 0x1D, 0x28, 0x0F, 0x36, 0xE3,
            0x23, 0xB4, 0x03, 0xD8, 0x90, 0xC8, 0x3C, 0xFE, 0x5E, 0x32, 0x24, 0x50, 0x1F, 0x3A, 0x43, 0x8A,
            0x96, 0x41, 0x74, 0xAC, 0x52, 0x33, 0xF0, 0xD9, 0x29, 0x80, 0xB1, 0x16, 0xD3, 0xAB, 0x91, 0xB9,
            0x84, 0x7F, 0x61, 0x1E, 0xCF, 0xC5, 0xD1, 0x56, 0x3D, 0xCA, 0xF4, 0x05, 0xC6, 0xE5, 0x08, 0x49 };

		byte newiv[4] = {0xF2, 0x53, 0x50, 0xC6};
		byte input;
		byte valueinput;
		dword fulliv;
		dword shift;

		for (byte i = 0; i < 4; i++) 
		{
				input = vector[i];
				valueinput = im12andwhatisthis[input];

				newiv[0] += (im12andwhatisthis[newiv[1]] - input);
				newiv[1] -= (newiv[2] ^ valueinput);
				newiv[2] ^= (im12andwhatisthis[newiv[3]] + input);
				newiv[3] -= (newiv[0] - valueinput);

				fulliv = (newiv[3] << 24) | (newiv[2] << 16) | (newiv[1] << 8) | newiv[0];
				shift = (fulliv >> 0x1D) | (fulliv << 0x03);

				newiv[0] = static_cast<byte>(shift & 0xFFu);
				newiv[1] = static_cast<byte>((shift >> 8) & 0xFFu);
				newiv[2] = static_cast<byte>((shift >> 16) & 0xFFu);
				newiv[3] = static_cast<byte>((shift >> 24) & 0xFFu);
		}

		for (byte i = 0; i < 4; i++)
			std::copy(newiv, newiv + 4, vector + i * 4);
	}

	void crypt::mapledecrypt(byte *buf, signed_dword size)
	{
		// I have no idea what I'm doing
		// this encryption shit was reversed from the game itself
		signed_dword j;
		byte a, b, c;

		for (byte i = 0; i < 3; i++)
		{
			a = 0;
			b = 0;

			for (j = size; j > 0; j--)
			{
				c = buf[j - 1];
				c = asmop::rol(c, 3);
				c = c ^ 0x13;
				a = c;
				c = c ^ b;
				c = (byte)(c - j); // shitty cast
				c = asmop::ror(c, 4);
				b = a;
				buf[j - 1] = c;
			}

			a = 0;
			b = 0;

			for (j = size; j > 0; j--)
			{
				c = buf[size - j];
				c = c - 0x48;
				c = c ^ 0xFF;
				c = asmop::rol(c, j);
				a = c;
				c = c ^ b;
				c = (byte)(c - j); // shitty cast
				c = asmop::ror(c, 3);
				b = a;
				buf[size - j] = c;
			}
		}
	}

	void crypt::maplecrypt(byte *buf, signed_dword size)
	{
		// I have no idea what I'm doing
		// this encryption shit was reversed from the game itself
		signed_dword j;
		byte a, c;

		for (byte i = 0; i < 3; i++)
		{
			a = 0;

			for (j = size; j > 0; j--)
			{
				c = buf[size - j];
				c = asmop::rol(c, 3);
				c = (byte)(c + j); // shitty cast
				c = c ^ a;
				a = c;
				c = asmop::ror(a, j);
				c = c ^ 0xFF;
				c = c + 0x48;
				buf[size - j] = c;
			}

			a = 0;

			for (j = size; j > 0; j--)
			{
				c = buf[j - 1];
				c = asmop::rol(c, 4);
				c = (byte)(c + j); // shitty cast
				c = c ^ a;
				a = c;
				c = c ^ 0x13;
				c = asmop::ror(c, 3);
				buf[j - 1] = c;
			}
		}
	}
}
