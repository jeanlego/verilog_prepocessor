/* Authors: Aaron Graham (aaron.graham@unb.ca, aarongraham9@gmail.com),
 *           Jean-Philippe Legault (jlegault@unb.ca, jeanphilippe.legault@gmail.com) and
 *            Dr. Kenneth B. Kent (ken@unb.ca)
 *            for the Reconfigurable Computing Research Lab at the
 *             Univerity of New Brunswick in Fredericton, New Brunswick, Canada
 */

#include <cstdarg>
#include <stdio.h>
#include <algorithm>
#include <string>
#include <vector>
#include <cctype>
#include <iostream>

#include "rtl_int.h"

// this sets the default bit width
#define DEFAULT_BIT_WIDTH 32
#define BRK brk_pt();
static void brk_pt(){
	return;
}

/***
 *     ___  __   __   __   __      __   ___  __   __   __  ___         __  
 *    |__  |__) |__) /  \ |__)    |__) |__  |__) /  \ |__)  |  | |\ | / _` 
 *    |___ |  \ |  \ \__/ |  \    |  \ |___ |    \__/ |  \  |  | | \| \__> 
 *                                                                         
 */
inline static RTL_INT return_internal_representation(bool sign, std::size_t length, std::string bitstring)
{
	RTL_INT to_return;
	to_return.push_back((sign)?"1":"0");
	to_return.push_back(std::to_string(length));
	to_return.push_back(bitstring);
	return to_return;
}

#define bad_value(test) _bad_value(static_cast<char>(std::tolower(test)), __func__, __LINE__); std::abort()
inline static char _bad_value(char test, const char *FUNCT, int LINE)	
{	
	std::cout << "INVALID BIT INPUT: (" << std::string(1,test) << ")@" << FUNCT << "::" << std::to_string(LINE) << std::endl;	
	return 'x'; 
}

/***
 *     __    ___     __  ___  __          __                     __        ___  __   __  
 *    |__) |  |     /__`  |  |__) | |\ | / _`    |__|  /\  |\ | |  \ |    |__  |__) /__` 
 *    |__) |  |     .__/  |  |  \ | | \| \__>    |  | /~~\ | \| |__/ |___ |___ |  \ .__/ 
 *                                                                                       
 */

#define MSB 0
#define get_bitstring(bits)			bits[2]
#define PUSH_MS_BITSTRING(str,chr)	str.insert(MSB,chr)
#define PUSH_MSB(str,chr)			str.insert(MSB,std::string(1,chr))

#define size_max(a,b) ((a) > (b)) ? (a): (b)
#define V_ZERO return_internal_representation(false,1,"0")
#define V_ONE return_internal_representation(false,1,"1")
#define V_UNK return_internal_representation(false,1,"x")

#define V_S_ZERO return_internal_representation(true,2,"00")
#define V_S_ONE return_internal_representation(true,2,"01")
#define V_S_UNK return_internal_representation(true,2,"xx")
#define V_S_NEG_ONE return_internal_representation(true,2,"11")

#define is_dont_care_string(input) (input.find("xXzZ") != std::string::npos)
#define assert_string_of_radix(input, radix) _assert_string_of_radix(input, radix, __func__,__LINE__)
inline static void _assert_string_of_radix(std::string input, std::size_t radix, const char *FUNCT, int LINE)
{
	BRK
	bool pass = false;
	switch(radix)
	{
		case 2:		pass = (input.find_first_not_of("xXzZ01") == std::string::npos);						break;
		case 8:		pass = (input.find_first_not_of("xXzZ01234567") == std::string::npos);					break;
		case 10:	pass = (input.find_first_not_of("0123456789") == std::string::npos);					break;
		case 16:	pass = (input.find_first_not_of("xZzZ0123456789aAbBcCdDeEfF") == std::string::npos);	break;
		default:	pass = false;	break;	
	}
	if(!pass)
	{
		std::cout << "Invalid bitstring of radix input " << std::to_string(radix) << " number: " << input << " @" << FUNCT << "::" << std::to_string(LINE) << std::endl;
		std::abort();
	}
}

#define v_strtoul(num,radix) _v_strtoul(num,radix,__func__, __LINE__)
inline static std::size_t _v_strtoul(std::string orig_number, std::size_t radix, const char *FUNCT, int LINE)
{
	BRK
	assert_string_of_radix(orig_number, radix);
	if (is_dont_care_string(orig_number))
	{
		std::cout << "Invalid Number contains dont care values. number: " << orig_number << " @" << FUNCT << "::" << std::to_string(LINE) << std::endl;
		std::abort();
	}
	else if( orig_number.length() >= (sizeof(long)*8) )
	{
		std::cout << "Invalid Number. Too large to be converted. number: " << orig_number << " upper limit: " << std::to_string((sizeof(long)*8)) << " @" << FUNCT << "::" << std::to_string(LINE) << std::endl;
		std::abort();
	}
	
	return std::strtoul(orig_number.c_str(),NULL,static_cast<int>(radix));
}
#define is_signed(bits)	(bits[0] == "0")? 0 : 1
#define is_negative(bits) (is_signed(bits) && get_bit(bits,MSB) == '1')
#define get_len(internal_bit_struct) _get_len(internal_bit_struct, __func__, __LINE__)
inline static std::size_t _get_len(RTL_INT& internal_bit_struct, const char *FUNCT, int LINE)
{
	BRK
	std::size_t defined_size = _v_strtoul(internal_bit_struct[1],10, FUNCT, LINE);
	std::size_t current_bit_width = internal_bit_struct[2].length();

	if(defined_size == 0)
	{
		//remove padding bits
		std::size_t loc = internal_bit_struct[2].find_first_not_of(internal_bit_struct[2][MSB]);
		if(loc != std::string::npos && loc)
			internal_bit_struct[2].erase(0,loc);

		return current_bit_width;
	}
	else
	{
		//expand to length
		if(defined_size > current_bit_width)
		{
			std::string padding_bits(defined_size-current_bit_width, (!is_signed(internal_bit_struct) || internal_bit_struct[2][MSB] =='x')? '0': internal_bit_struct[2][MSB]);
			internal_bit_struct[2].insert(MSB,padding_bits);
		}

		//truncate to length
		else if (defined_size < current_bit_width)
			internal_bit_struct[2].erase(MSB,current_bit_width-defined_size);

		return defined_size;
	}
}

#define get_bit(int_bits, location) _get_bit(int_bits, location, __func__, __LINE__)
inline static char _get_bit(RTL_INT internal_bit_struct, std::size_t location, const char *FUNCT, int LINE)
{
	BRK
	if(location >= get_len(internal_bit_struct))
	{
		std::clog << "INVALID INPUT BITSTRING INDEX: (" << std::to_string(location) << ")@" << FUNCT << "::" << std::to_string(LINE) << std::endl;	
		std::abort();	
	}
	return static_cast<char>(internal_bit_struct[2][location]);
}

#define is_signed(bits)	(bits[0] == "0")? 0 : 1
inline static RTL_INT resize(RTL_INT internal_bit_struct, std::size_t len)
{
	internal_bit_struct[1] = std::to_string(len);
	get_len(internal_bit_struct);
	return internal_bit_struct;
}

//resize bitstring to its own length
inline static RTL_INT readjust_size(RTL_INT internal_bit_struct)
{
	BRK
	get_len(internal_bit_struct);
	return internal_bit_struct;
}

/**********************
 * convert from and to internal representation bitstring
 */
inline static std::string to_bitstring(std::string orig_string, std::size_t radix)
{
	BRK
	std::string result = "";	
	assert_string_of_radix(orig_string,radix);
	while(orig_string != "")
	{
		switch(radix)
		{
			case 10:
			{
				std::string new_number = "";
				short prev = '0';
				short flag = 0;
				for(size_t i=orig_string.length()-1; i<orig_string.length(); i--)
				{
					short current_digit = (orig_string[i]-'0');
					short new_pair = (prev%2)*10 + current_digit;
					std::string div = std::to_string(new_pair/2);
					std::string rem = std::to_string(new_pair%2);

					prev = current_digit;
					if(div != "0" && !flag)
						flag = 1;

					if(flag)
						new_number.append(div);
					
					if(i == MSB)
					{
						PUSH_MS_BITSTRING(result, rem);
						orig_string = new_number;
					}
				}
				break;
			}
			case 2:		//fallthrough
			case 8:		//fallthrough
			case 16:
			{
				int lsb = std::tolower(orig_string.back()); 
				orig_string.pop_back();
				switch(radix)
				{
					case 2:
					{
						switch(lsb)
						{
							case '0': PUSH_MS_BITSTRING(result, "0");	break;
							case '1': PUSH_MS_BITSTRING(result, "1");	break;
							case 'x': PUSH_MS_BITSTRING(result, "x");	break;
							case 'z': PUSH_MS_BITSTRING(result, "z");	break;
							default:  bad_value(lsb);
						}
						break;
					}
					case 8:
					{
						switch(lsb)
						{
							case '0': PUSH_MS_BITSTRING(result, "000");	break;
							case '1': PUSH_MS_BITSTRING(result, "001");	break;
							case '2': PUSH_MS_BITSTRING(result, "010");	break;
							case '3': PUSH_MS_BITSTRING(result, "011");	break;
							case '4': PUSH_MS_BITSTRING(result, "100");	break;
							case '5': PUSH_MS_BITSTRING(result, "101");	break;
							case '6': PUSH_MS_BITSTRING(result, "110");	break;
							case '7': PUSH_MS_BITSTRING(result, "111");	break;
							case 'x': PUSH_MS_BITSTRING(result, "xxx");	break;
							case 'z': PUSH_MS_BITSTRING(result, "zzz");	break;
							default:  bad_value(lsb);
						}
						break;
					}
					case 16:
					{
						switch(lsb)
						{
							case '0': PUSH_MS_BITSTRING(result, "0000");	break;
							case '1': PUSH_MS_BITSTRING(result, "0001");	break;
							case '2': PUSH_MS_BITSTRING(result, "0010");	break;
							case '3': PUSH_MS_BITSTRING(result, "0011");	break;
							case '4': PUSH_MS_BITSTRING(result, "0100");	break;
							case '5': PUSH_MS_BITSTRING(result, "0101");	break;
							case '6': PUSH_MS_BITSTRING(result, "0110");	break;
							case '7': PUSH_MS_BITSTRING(result, "0111");	break;
							case '8': PUSH_MS_BITSTRING(result, "1000");	break;
							case '9': PUSH_MS_BITSTRING(result, "1001");	break;
							case 'a': PUSH_MS_BITSTRING(result, "1010");	break;
							case 'b': PUSH_MS_BITSTRING(result, "1011");	break;
							case 'c': PUSH_MS_BITSTRING(result, "1100");	break;
							case 'd': PUSH_MS_BITSTRING(result, "1101");	break;
							case 'e': PUSH_MS_BITSTRING(result, "1110");	break;
							case 'f': PUSH_MS_BITSTRING(result, "1111");	break;
							case 'x': PUSH_MS_BITSTRING(result, "xxxx");	break;
							case 'z': PUSH_MS_BITSTRING(result, "zzzz");	break;
							default:  bad_value(lsb);
						}
						break;
					}
					default:
					{
						std::cout << "Invalid base " << std::to_string(radix) << " number: " << orig_string << ".\n";
						std::abort();
					}
				}
				break;
			}
			default:
			{
				std::cout << "Invalid base " << std::to_string(radix) << " number: " << orig_string << ".\n";
				std::abort();
			}
		}
	}
	return result;
}

RTL_INT standardize_input(std::string input_number)
{	
	BRK
	//remove underscores
	input_number.erase(std::remove(input_number.begin(), input_number.end(), '_'), input_number.end());

	std::size_t loc = input_number.find("\'");
	if(loc == std::string::npos)
	{
		input_number.insert(0, "\'d");
		loc = 0;
	}

	std::size_t len = DEFAULT_BIT_WIDTH;
	if(loc != 0)
		len = v_strtoul(input_number.substr(0,loc),10);

	bool sign = false;
	if(std::tolower(input_number[loc+1]) == 's')
		sign = true;
	
	std::string result = "";
	switch(std::tolower(input_number[loc+1+sign]))
	{
		case 'b':	result = to_bitstring(input_number.substr(loc+2+sign), 2); break;
		case 'o':	result = to_bitstring(input_number.substr(loc+2+sign), 8); break;
		case 'd':	result = to_bitstring(input_number.substr(loc+2+sign), 10); break;
		case 'h':	result = to_bitstring(input_number.substr(loc+2+sign), 16); break;
		default:
		{
			std::cout << "Invalid base " << std::string(1,input_number[loc+1]) << " number: " << input_number << ".\n";
			std::abort();
		}
	}
	RTL_INT output = return_internal_representation(sign, len, result);
	output = resize(output, len);
	return output;
}

// convert internal format to verilog
std::string v_bin_string(RTL_INT internal_binary_number)
{
	BRK
	//final resize
	readjust_size(internal_binary_number);

	std::string output = "";

	if(internal_binary_number[1] != "0")
		output += internal_binary_number[1];

	output += "\'";

	if(is_signed(internal_binary_number)) 
		output += "s";

	return output + "b" + internal_binary_number[2];
}

/***
 *     __   __            ___         ___     __   __   ___  __       ___    __       
 *    |__) |__) |  |\/| |  |  | \  / |__     /  \ |__) |__  |__)  /\   |  | /  \ |\ | 
 *    |    |  \ |  |  | |  |  |  \/  |___    \__/ |    |___ |  \ /~~\  |  | \__/ | \| 
 *                                                                                    
 *                                                                
 *                                                                  
 * these are taken from the raw verilog truth tables so that the evaluation are correct.
 * only use this to evaluate any expression for the number_t binary digits.
 * reference: http://staff.ustc.edu.cn/~songch/download/IEEE.1364-2005.pdf
 * 
 *******************************************************/

static const char l_buf[4] = {
	/*	 0   1   x   z  <- a*/
		'0','1','x','x'
};

static const char l_not[4] = {
	/*   0   1   x   z 	<- a */
		'1','0','x','x'
};

static const char l_and[4][4] = {
	/* a  /	 0   1   x   z 	<-b */	
	/* 0 */	{'0','0','0','0'},	
	/* 1 */	{'0','1','x','x'},	
	/* x */	{'0','x','x','x'},	
	/* z */	{'0','x','x','x'}
};

static const char l_or[4][4] = {
	/* a  /	 0   1   x   z 	<-b */	
	/* 0 */	{'0','1','x','x'},	
	/* 1 */	{'1','1','1','1'},	
	/* x */	{'x','1','x','x'},	
	/* z */	{'x','1','x','x'}
};

static const char l_xor[4][4] = {
	/* a  /	 0   1   x   z 	<-b */	
	/* 0 */	{'0','1','x','x'},	
	/* 1 */	{'1','0','x','x'},	
	/* x */	{'x','x','x','x'},	
	/* z */	{'x','x','x','x'}
};

static const char l_nand[4][4] = {
	/* a  /	 0   1   x   z 	<-b */	
	/* 0 */	{'1','1','1','1'},	
	/* 1 */	{'1','0','x','x'},	
	/* x */	{'1','x','x','x'},	
	/* z */	{'1','x','x','x'}
};

static const char l_nor[4][4] = {
	/* a  /	 0   1   x   z 	<-b */	
	/* 0 */	{'1','0','x','x'},	
	/* 1 */	{'0','0','0','0'},	
	/* x */	{'x','0','x','x'},	
	/* z */	{'x','0','x','x'}
};

static const char l_xnor[4][4] = {
	/* a  /	 0   1   x   z 	<-b */	
	/* 0 */	{'1','0','x','x'},	
	/* 1 */	{'0','1','x','x'},	
	/* x */	{'x','x','x','x'},	
	/* z */	{'x','x','x','x'}
};

static const char l_notif1[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'z','1','H','H'},
	/* 1 */	{'z','0','L','L'},
	/* x */	{'z','x','x','x'},
	/* z */	{'z','x','x','x'}
};

static const char l_notif0[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'1','z','H','H'},
	/* 1 */	{'0','z','L','L'},
	/* x */	{'x','z','x','x'},
	/* z */	{'x','z','x','x'}
};

static const char l_bufif1[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'z','0','H','H'},
	/* 1 */	{'z','1','L','L'},
	/* x */	{'z','x','x','x'},
	/* z */	{'z','x','x','x'}
};

static const char l_bufif0[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'0','z','H','H'},
	/* 1 */	{'1','z','L','L'},
	/* x */	{'x','z','x','x'},
	/* z */	{'x','z','x','x'}
};

static const char l_rpmos[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'0','z','L','L'},
	/* 1 */	{'1','z','H','H'},
	/* x */	{'x','z','x','x'},
	/* z */	{'z','z','z','z'}
};

static const char l_rnmos[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'z','0','L','L'},
	/* 1 */	{'z','1','H','H'},
	/* x */	{'z','x','x','x'},
	/* z */	{'z','z','z','z'}
};

static const char l_nmos[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'z','0','L','L'},
	/* 1 */	{'z','1','H','H'},
	/* x */	{'z','x','x','x'},
	/* z */	{'z','z','z','z'}
};

// see table 5-21 p:54 IEEE 1364-2005
static char l_ternary[4][4] = {
	/* in /	 0   1   x   z 	<-control */	
	/* 0 */	{'0','x','x','x'},
	/* 1 */	{'x','1','x','x'},
	/* x */	{'x','x','x','x'},
	/* z */	{'x','x','x','x'}
};

#define REF_0 0
#define REF_1 1
#define REF_x 2
#define REF_z 3

#define v_unary_op(a,op) _v_unary_op(a, op, __func__, __LINE__)
inline static char _v_unary_op(const char a, const char lut[4], const char *FUNCT, int LINE) 
{
	return 	((a) == '0')				?	lut[REF_0] :
			((a) == '1')				?	lut[REF_1] :
			(std::tolower(a) == 'x')	?	lut[REF_x] : 
			(std::tolower(a) == 'z')	?	lut[REF_z] :
											_bad_value(a, FUNCT,LINE)
			;
}

#define v_binary_op(a,b,op)	_v_binary_op(a, b, op, __func__, __LINE__)
inline static char _v_binary_op(const char a, const char b, const char lut[4][4], const char *FUNCT, int LINE) 
{
	return 	((a) == '0')				? 	_v_unary_op(b, lut[REF_0], FUNCT, LINE)	:
			((a) == '1')				? 	_v_unary_op(b, lut[REF_1], FUNCT, LINE) :
			(std::tolower(a) == 'x')	?	_v_unary_op(b, lut[REF_x], FUNCT, LINE)	: 
			(std::tolower(a) == 'z')	?	_v_unary_op(b, lut[REF_z], FUNCT, LINE) :
											_bad_value(a, FUNCT, LINE)
			;
}

#define v_sum(a,b,c)	_v_sum(a, b, c, __func__, __LINE__)
inline static char _v_sum(const char a, const char b, const char c, const char *FUNCT, int LINE) 
{
	return 	((a) == '0')				? 	_v_binary_op(b,c, l_xor, FUNCT, LINE)	:
			((a) == '1')				? 	_v_binary_op(b,c, l_xnor, FUNCT, LINE) :
			(std::tolower(a) == 'x')	?	'x'	: 
			(std::tolower(a) == 'z')	?	'x' :
											_bad_value(a, FUNCT, LINE)
			;
}

#define v_carry(a,b,c)	_v_carry(a, b, c, __func__, __LINE__)
inline static char _v_carry(const char a, const char b, const char c, const char *FUNCT, int LINE) 
{
	return 	((a) == '0')				? 	_v_binary_op(b,c, l_and, FUNCT, LINE)	:
			((a) == '1')				? 	_v_binary_op(b,c, l_or, FUNCT, LINE) :
			(std::tolower(a) == 'x')	?	_v_binary_op(b,c, l_ternary, FUNCT, LINE)	: 
			(std::tolower(a) == 'z')	?	_v_binary_op(b,c, l_ternary, FUNCT, LINE) :
											_bad_value(a, FUNCT, LINE)
			;
}

/***
 * these are extended defines to simplify our lives
 */
// #define v_buf(a)				v_unary_op(a, 				l_buf)
// #define v_not(a)				v_unary_op(a, 				l_not)
// #define v_and(a,b)				v_binary_op(a,b, 			l_and)
// #define v_or(a,b)				v_binary_op(a,b, 			l_or)
// #define v_xor(a,b)				v_binary_op(a,b, 			l_xor)
// #define v_nand(a,b)				v_binary_op(a,b, 			l_nand)
// #define v_nor(a,b)				v_binary_op(a,b, 			l_nor)
// #define v_xnor(a,b)				v_binary_op(a,b, 			l_xnor)
// #define v_notif1(in,control)	v_binary_op(in,control, 	l_notif1)
// #define v_notif0(in,control)	v_binary_op(in,control, 	l_notif0)
// #define v_bufif1(in,control)	v_binary_op(in,control, 	l_bufif1)
// #define v_bufif0(in,control)	v_binary_op(in,control, 	l_bufif0)
// #define v_rpmos(in, control)	v_binary_op(in,control, 	l_rpmos)
// #define v_pmos(in,control)		v_binary_op(in,control, 	l_pmos)
// #define v_rnmos(in, control)	v_binary_op(in,control, 	l_rnmos)
// #define v_nmos(in,control)		v_binary_op(in,control, 	l_nmos)
// #define v_ternary(in,control)	v_binary_op(in,control,		l_ternary)

// #define v_sum(a, b, carry)		v_three_op(a,b,carry,		l_sum)
// #define v_carry(a, b, carry)	v_three_op(a,b,carry,		l_carry)


//TODO: signed numbers!

EVAL_RESULT return_int_eval(RTL_INT a,RTL_INT b)
{
	std::size_t std_length = std::max(get_len(a),get_len(b));
	a = resize(a, std_length);
	b = resize(b, std_length);
	
	bool signed_result = is_signed(a) && is_signed(b);
	bool neg_a = (signed_result && is_negative(a));
	bool neg_b = (signed_result && is_negative(b));
	
	if(neg_a)	a = V_MINUS(a);
	if(neg_b)	b = V_MINUS(a);
	bool invert_result = ((!neg_a && neg_b) || (neg_a && !neg_b));

	auto bit_a = get_bitstring(a).begin();
	auto bit_b = get_bitstring(b).begin();
	for (; bit_a != get_bitstring(a).end() && bit_b != get_bitstring(b).end(); ++bit_a, ++bit_b)
	{
		if		(*bit_a == '1' && *bit_b == '0')	return (invert_result)? LESS_THAN: GREATHER_THAN;
		else if	(*bit_a == '0' && *bit_b == '1')	return (invert_result)? GREATHER_THAN: LESS_THAN;
		else if	(*bit_a == 'x' || *bit_b == 'x')	return UNKNOWN;
	}
	return EQUAL;
}
EVAL_RESULT return_int_eval(long a,RTL_INT b)
{
	return return_int_eval(standardize_input(std::to_string(a)),b);
}
EVAL_RESULT return_int_eval(RTL_INT a,long b)
{
	return return_int_eval(a,standardize_input(std::to_string(b)));
}


//LSB -> MSB
inline static std::string V_REDUX(std::string& a, const char lut[4])
{
	std::string result = "";
	for (auto bit_a = a.crbegin(); bit_a != a.crend(); ++bit_a)
		PUSH_MSB(result, v_unary_op(*bit_a,lut));

	return result;
}
inline static RTL_INT V_REDUX(RTL_INT a, const char lut[4])
{
	std::size_t len =  get_len(a);
	return return_internal_representation(is_signed(a), len, V_REDUX(get_bitstring(a),lut));
}

inline static signed char V_REDUX(std::string& a, const char lut[4][4])
{
	signed char result = 0;
	auto bit_a = a.crbegin();
    for (; bit_a != a.crend(); ++bit_a)
		result = (!result)	?	*bit_a:	v_binary_op(*bit_a, result, lut);

	return result;
}
inline static RTL_INT V_REDUX(RTL_INT a, const char lut[4][4])
{
	return return_internal_representation(false, 1, std::string(1,V_REDUX(get_bitstring(a),lut)));
}

inline static std::string V_REDUX(std::string& a, std::string& b, const char lut[4][4])
{
	std::string result = "";

	for (auto bit_a = a.crbegin(), bit_b = b.crbegin(); bit_a != a.crend() && bit_b != b.crend(); ++bit_a, ++bit_b)
		PUSH_MSB(result, v_binary_op(*bit_a, *bit_b,lut));

	return result;
}
inline static RTL_INT V_REDUX(RTL_INT a, RTL_INT b, const char lut[4][4])
{
	std::size_t std_length = size_max(get_len(a), get_len(b));
	a = resize(a, std_length);
	b = resize(b, std_length);

	return return_internal_representation(false, std_length, V_REDUX(get_bitstring(a), get_bitstring(b),lut));
}

inline static std::string V_INCREMENT(std::string a, const char lut_adder[4][4], const char lut_carry[4][4], signed const char initial)
{
	std::string result = "";
	char tmp_carry  = initial;
	
	auto bit_a = a.crbegin();
    for (; bit_a != a.crend(); ++bit_a) {
		PUSH_MSB(result, v_binary_op(*bit_a,tmp_carry, lut_adder));
		tmp_carry = v_binary_op(*bit_a, tmp_carry, lut_carry);
	}
	return result;
}
inline static RTL_INT V_INCREMENT(RTL_INT a, const char lut_adder[4][4], const char lut_carry[4][4], signed const char initial)
{
	return return_internal_representation(is_signed(a), get_len(a), V_INCREMENT(get_bitstring(a), lut_adder, lut_carry, initial));
}

/***
 *                    __          __   __   ___  __       ___    __       
 *    |  | |\ |  /\  |__) \ /    /  \ |__) |__  |__)  /\   |  | /  \ |\ | 
 *    \__/ | \| /~~\ |  \  |     \__/ |    |___ |  \ /~~\  |  | \__/ | \| 
 *                                                                        
 */

RTL_INT V_NEG(RTL_INT a)
{
	return V_REDUX(a,l_not);
}

RTL_INT V_PLUS_PLUS(RTL_INT a)
{
	return V_INCREMENT(a, l_xor, l_and, '1');
}

RTL_INT  V_MINUS_MINUS(RTL_INT a)
{
	return V_INCREMENT(a, l_xnor, l_or, '0');
}

RTL_INT V_ADD(RTL_INT a)
{
	return a;
}

RTL_INT V_MINUS(RTL_INT a)
{
	return V_PLUS_PLUS(V_NEG(a));
}

RTL_INT V_AND(RTL_INT a)
{
	return V_REDUX(a, l_and);
}

RTL_INT V_OR(RTL_INT a)
{
	return V_REDUX(a, l_or);	
}

RTL_INT V_XOR(RTL_INT a)
{
	return V_REDUX(a, l_xor);
}

RTL_INT V_NAND(RTL_INT a)
{
	return V_NEG(V_AND(a));
}

RTL_INT V_NOR(RTL_INT a)
{
	return V_NEG(V_OR(a));
}

RTL_INT V_XNOR(RTL_INT a)
{
	return V_NEG(V_XOR(a));
}

RTL_INT V_LOGICAL_NOT(RTL_INT a)
{
	return V_NEG(V_OR(a));
}

/***
 *     __               __          __   __   ___  __       ___    __       
 *    |__) | |\ |  /\  |__) \ /    /  \ |__) |__  |__)  /\   |  | /  \ |\ | 
 *    |__) | | \| /~~\ |  \  |     \__/ |    |___ |  \ /~~\  |  | \__/ | \| 
 *                                                                          
 */

RTL_INT V_AND(RTL_INT a,RTL_INT b)
{
	return V_REDUX(a,b,l_and);
}

RTL_INT V_OR(RTL_INT a,RTL_INT b)
{
	return V_REDUX(a,b,l_or);
}

RTL_INT V_XOR(RTL_INT a,RTL_INT b)
{
	return V_REDUX(a,b,l_xor);
}

RTL_INT V_NAND(RTL_INT a,RTL_INT b)
{
	return V_REDUX(a,b,l_nand);
}

RTL_INT V_NOR(RTL_INT a,RTL_INT b)
{
	return V_REDUX(a,b,l_nor);
}

RTL_INT V_XNOR(RTL_INT a,RTL_INT b)
{
	return V_REDUX(a,b,l_xnor);
}

RTL_INT V_CASE_EQUAL(RTL_INT a,RTL_INT b)
{
	std::size_t std_length = size_max(get_len(a), get_len(b));
	a = resize(a, std_length);
	b = resize(b, std_length);

	auto bit_a = get_bitstring(a).crbegin();
	auto bit_b = get_bitstring(b).crbegin();
	for (; bit_a != get_bitstring(a).crend() && bit_b != get_bitstring(b).crend(); ++bit_a, ++bit_b)
		if(*bit_a != *bit_b)	return V_ZERO;
	
	return V_ONE;
}
RTL_INT V_CASE_NOT_EQUAL(RTL_INT a,RTL_INT b)
{
	return V_LOGICAL_NOT(V_CASE_EQUAL(a,b));
}

/**
 * Shift operations
 */

inline static void shift_op(std::string& bit_string, int len, signed char padding_bit)
{
	std::string pad(std::abs(len),padding_bit);
	//shift left , let it grow, let it grow ...
	if(len > 0)	
	{
		bit_string.append(pad);
	}
	//shift right, because it's the rightest thing to do
	else if(len < 0)
	{
		bit_string.erase(bit_string.length()+len);
		bit_string.insert(MSB,pad);
	}
}
RTL_INT V_SIGNED_SHIFT_LEFT(RTL_INT a, RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(b)))	
		return V_UNK;
	
	std::size_t len = v_strtoul(get_bitstring(b),2);
	shift_op(get_bitstring(a), len, '0');
	a = resize(a,0);
	a = resize(a,get_len(a));
	return a;
}

RTL_INT V_SHIFT_LEFT(RTL_INT a, RTL_INT b)
{
	return V_SIGNED_SHIFT_LEFT(a,b);
}

RTL_INT V_SIGNED_SHIFT_RIGHT(RTL_INT a, RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(b)))	
		return V_UNK;
	
	shift_op(get_bitstring(a), -1 *v_strtoul(get_bitstring(b),2), get_bit(a,MSB));
	return a;
}

RTL_INT V_SHIFT_RIGHT(RTL_INT a, RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(b)))	
		return V_UNK;

	shift_op(get_bitstring(a), -1 *v_strtoul(get_bitstring(b),2), '0');
	return a;

}

/**
 * Logical Operations
 */
RTL_INT V_LOGICAL_AND(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return V_AND(V_OR(a),V_OR(b));
}

RTL_INT V_LOGICAL_OR(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return V_OR(V_OR(a),V_OR(b));
}

RTL_INT V_LT(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return	return_internal_representation(false, 1, (return_int_eval(a,b) == LESS_THAN) 	? "1":"0");
}

RTL_INT V_GT(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return	return_internal_representation(false, 1, (return_int_eval(a,b) == GREATHER_THAN) 	? "1":"0");
}

RTL_INT V_LE(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return	return_internal_representation(false, 1, (return_int_eval(a,b) != GREATHER_THAN) 	? "1":"0");
}

RTL_INT V_GE(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return	return_internal_representation(false, 1, (return_int_eval(a,b) != LESS_THAN) 	? "1":"0");
}

RTL_INT V_EQUAL(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return	return_internal_representation(false, 1, (return_int_eval(a,b) == EQUAL) 	? "1":"0");
}

RTL_INT V_NOT_EQUAL(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	return	return_internal_representation(false, 1, (return_int_eval(a,b) != EQUAL) 	? "1":"0");
}

inline static std::string add_internal(std::string& a, std::string& b)
{

	char previous_carry = '0';
	std::string result = "";
	auto bit_a = a.crbegin();
	auto bit_b = b.crbegin();
	for (; bit_a != a.crend() && bit_b != b.crend(); ++bit_a, ++bit_b)
	{
		PUSH_MSB(result,v_sum(*bit_a, *bit_b, previous_carry));
		previous_carry = v_carry(*bit_a, *bit_b, previous_carry);
	}
	PUSH_MSB(result,previous_carry);
	return result;
}

RTL_INT V_ADD(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_UNK;

	std::size_t std_length = size_max(get_len(a), get_len(b));
	a = resize(a, std_length);
	b = resize(b, std_length);
	RTL_INT output = return_internal_representation(is_signed(a) && is_signed(b), std_length, add_internal(get_bitstring(a),get_bitstring(b)));
	get_len(output);
	return	output;
}

RTL_INT V_MINUS(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_S_UNK;

	return V_ADD(a, V_MINUS(b));
}

RTL_INT V_MULTIPLY(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_S_UNK;

	bool signed_result = is_signed(a) && is_signed(b);
	bool neg_a = (signed_result && is_negative(a));
	bool neg_b = (signed_result && is_negative(b));
	
	if(neg_a)	a = V_MINUS(a);
	if(neg_b)	b = V_MINUS(a);
	bool invert_result = ((!neg_a && neg_b) || (neg_a && !neg_b));


	std::size_t std_length = std::max(get_len(b),get_len(a))*2;
	RTL_INT result = return_internal_representation(is_signed(a) && is_signed(b),std_length,"0");

	for (auto bit_a = get_bitstring(a).crbegin(); bit_a != get_bitstring(a).crend(); ++bit_a)
	{
		if(*bit_a == '1')
			result = V_ADD(result,b);
		shift_op(get_bitstring(b), 1, '0');
	}

	if(invert_result)	
		result = V_MINUS(result);
	// TODO what size do we go to ??
	return result;
}

RTL_INT V_POWER(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b)))
		return V_S_UNK;
	
	EVAL_RESULT res_a = return_int_eval(a, 0);
	short val_a = 	(res_a == EQUAL) 			? 	0: 
					(res_a == LESS_THAN) 		? 	((return_int_eval(a,-1) == LESS_THAN))	?	-2: -1:
					/* GREATHER_THAN */  			((return_int_eval(a,1) == GREATHER_THAN))	?	2: 1;

	EVAL_RESULT res_b = return_int_eval(b, 0);
	short val_b = 	(res_b == EQUAL) 			? 	0: 
					(res_b == LESS_THAN) 		? 	-1:
					/* GREATHER_THAN */				1;

	//compute
	if(val_b == 1 && (val_a < -1 || val_a > 1 ))
	{
		RTL_INT result = return_internal_representation(is_signed(a) && is_signed(b),0,"1");
		while(return_int_eval(b,0) == GREATHER_THAN)
		{
			b = V_MINUS_MINUS(b);
			result = V_MULTIPLY( result, a);
		}
		return result;
	}
	if (val_b == 0 || val_a == 1)	
	{
		return V_S_ONE;
	}
	else if(val_b == -1 && val_a == 0)
	{
		return V_S_UNK;
	}
	else if(val_a == -1)
	{
		if(get_bit(a,get_len(a)-1) == '0') 	// even
			return V_S_ONE;
		else								//	odd
			return V_S_NEG_ONE;
	}
	else	
	{
		return V_S_ZERO;
	}
}
RTL_INT V_DIV(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b))
	|| return_int_eval(b,0) == EQUAL)
		return V_UNK;

	
	RTL_INT result = return_internal_representation(is_signed(a) && is_signed(b),0,"0");
	//TODO signed division!
	while(return_int_eval(a, b) == GREATHER_THAN )
	{
		RTL_INT count = return_internal_representation(is_signed(a) && is_signed(b),0,"1");
		RTL_INT sub_with = b;
		RTL_INT tmp = b;
		while(return_int_eval(tmp, a) == LESS_THAN)
		{
			sub_with = tmp;
			shift_op(get_bitstring(count), 1,'0');
			shift_op(get_bitstring(tmp), 1,'0');
		}
		a = V_MINUS(a, sub_with);
		result = V_ADD(result, count);
	}
	return result;
}

RTL_INT V_MOD(RTL_INT a,RTL_INT b)
{
	if(is_dont_care_string(get_bitstring(a)) || is_dont_care_string(get_bitstring(b))
	|| return_int_eval(b, 0) == EQUAL)
		return V_UNK;

	//TODO signed division!
	while(return_int_eval(b, a)  == LESS_THAN)
	{
		RTL_INT sub_with = b;
		for( RTL_INT tmp = b; return_int_eval(tmp, a) == LESS_THAN; shift_op(get_bitstring(tmp), 1,'0'))
		{
			sub_with = tmp;
		}
		a = V_MINUS(a, sub_with);
	}
	return a;
}

/***
 *    ___  ___  __             __          __   __   ___  __       ___    __       
 *     |  |__  |__) |\ |  /\  |__) \ /    /  \ |__) |__  |__)  /\   |  | /  \ |\ | 
 *     |  |___ |  \ | \| /~~\ |  \  |     \__/ |    |___ |  \ /~~\  |  | \__/ | \| 
 *                                                                                 
*/
RTL_INT V_TERNARY(RTL_INT a, RTL_INT b, RTL_INT c)
{
	/*	if a evaluates properly	*/
	EVAL_RESULT eval = return_int_eval(V_LOGICAL_NOT(a),V_ZERO);
	if(eval == UNKNOWN)		return V_REDUX(b,c,l_ternary);
	else if(eval == EQUAL)	return b;
	else					return c;
}
