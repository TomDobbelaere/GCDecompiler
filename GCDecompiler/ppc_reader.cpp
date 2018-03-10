
#include <limits.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <set>
#include "types.h"
#include "rel.h"
#include "utils.h"

using std::string;
using std::ios;
using std::endl;

namespace PPC {

	static char instruction[4];
	static std::stringstream stream;
	static std::vector<string> args;

	static char opcode;
	static int LI, r1, r2, r3, r4, r5, BD, SIMM;
	static uint position, decider, e_decider, d, UIMM, crm;
	static bool AA, LK, OE, RC, func_end, q1, q2, q3;
	static string i_type;

	static bool get_bit(char pos) {
		pos -= 1;
		int loc = (int)floor(pos / 8);
		return instruction[loc] & (int)pow(2, 7 - (pos % 8));
	}

	static uint get_range(char start, char end) {
		uint out = 0;
		char num_bits = (end - start) + 1;
		for (char i = start, j = 1; i <= end; i++, j++) {
			out += (uint)pow(2, (num_bits - j))*get_bit(i);
		}
		return out;
	}

	static int get_signed_range(char start, char end) {
		uint value = get_range(start, end);
		char num_bits = end - start;
		uint mask = 1 << num_bits;
		return -((int)(value & mask)) + (int)(value & (mask - 1));
	}

	static void add_arg() {
		args.push_back(stream.str());
		stream.clear();
		stream.str("");
	}

	static void arg_1() {
		stream << get_range(7, 11);
		add_arg();
	}

	static void arg_r1() {
		stream << "r";
		arg_1();
	}

	static void arg_fr1() {
		stream << "fr";
		arg_1();
	}

	static void arg_crb1() {
		stream << "crb";
		arg_1();
	}

	static void arg_2() {
		stream << get_range(12, 16);
		add_arg();
	}

	static void arg_r2() {
		stream << "r";
		arg_2();
	}

	static void arg_fr2() {
		stream << "fr";
		arg_2();
	}

	static void arg_crb2() {
		stream << "crb";
		arg_2();
	}

	static void arg_3() {
		stream << get_range(17, 21);
		add_arg();
	}

	static void arg_r3() {
		stream << "r";
		arg_3();
	}

	static void arg_fr3() {
		stream << "fr";
		arg_3();
	}

	static void arg_crb3() {
		stream << "crb";
		arg_3();
	}

	static void arg_4() {
		stream << get_range(22, 26);
		add_arg();
	}

	static void arg_fr4() {
		stream << "fr";
		arg_4();
	}

	static void arg_5() {
		stream << get_range(27, 31);
		add_arg();
	}

	static void arg_crf1() {
		stream << "crf" << get_range(7, 9);
		add_arg();
	}

	static void arg_crf2() {
		stream << "crf" << get_range(12, 14);
		add_arg();
	}

	static void arg_BO() {
		arg_1();
	}

	static void arg_BI() {
		arg_2();
	}

	static void arg_BD() {
		stream << itoh(BD);
		add_arg();
	}

	static void arg_SIMM() {
		stream << itoh(SIMM);
		add_arg();
	}

	static void arg_UIMM() {
		stream << itoh(UIMM);
		add_arg();
	}

	static void arg_d() {
		stream << itoh(d);
		add_arg();
	}

	static void arg_SR() {
		stream << get_range(13, 16);
		add_arg();
	}

	static void arg_dr2() {
		stream << itoh(d) << "(r" << r2 << ")";
		add_arg();
	}

	static void arg_SIMMr2() {
		stream << itoh(SIMM) << "(r" << r2 << ")";
		add_arg();
	}

	void disassemble(string file_in, string file_out, int start, int end) {
		std::cout << "Disassembling PPC" << endl;
		std::fstream input(file_in, ios::in | ios::binary);
		std::fstream output(file_out, ios::out);

		q1 = false;
		q2 = false;
		q3 = false;

		if (end == -1) {
			input.seekg(0, ios::end);
			end = (int)input.tellg();
		}
		int size = end - start;
		int hex_length = (int)std::floor((std::log(size) / std::log(16)) + 1) + 2;

		input.seekg(start, ios::beg);
		while (input.tellg() < end) {
			position = (int)input.tellg() - start;
			
			if ((float)position / size > .25 && !q1) {
				std::cout << "  25% Complete" << endl;
				q1 = true;
			} else if ((float)position / size > .5 && !q2) {
				std::cout << "  50% Complete" << endl;
				q2 = true;
			} else if ((float)position / size > .75 && !q3) {
				std::cout << "  75% Complete" << endl;
				q3 = true;
			}

			input.read(instruction, 4);

			opcode = get_range(1, 6);
			decider = get_range(23, 31);
			e_decider = get_range(22, 31);
			OE = get_bit(22);
			AA = get_bit(31);
			RC = LK = get_bit(32);
			LI = get_range(7, 30);
			r1 = get_range(7, 11);
			r2 = get_range(12, 16);
			r3 = get_range(17, 21);
			r4 = get_range(22, 26);
			r5 = get_range(27, 31);
			BD = get_range(17, 30);
			crm = get_range(13, 20);
			SIMM = get_signed_range(17, 32);
			d = UIMM = get_range(17, 32);

			args.clear();
			i_type = "";
			
			switch (opcode){
			case 3:
				i_type = "twi";
				arg_1();
				arg_r2();
				arg_SIMM();
				break;
			case 7:
				i_type = "mulli";
				arg_r1();
				arg_r2();
				arg_SIMM();
				break;
			case 8:
				i_type = "subfic";
				arg_r1();
				arg_r2();
				arg_SIMM();
				break;
			case 10:
				if (get_bit(11)) {
					i_type = "cmpli";
				} else {
					i_type = "cmplwi";
				}
				arg_crf1();
				if (get_bit(11)) {
					stream << "1";
					add_arg();
				}
				arg_r2();
				arg_UIMM();
				break;
			case 11:
				if (get_bit(11)) {
					i_type = "cmpi";
				} else {
					i_type = "cmpwi";
				}
				arg_crf1();
				if (get_bit(11)) {
					stream << "1";
					add_arg();
				}
				arg_r2();
				arg_SIMM();
				break;
			case 12:
				i_type = "addic";
				arg_r1();
				arg_r2();
				arg_SIMM();
				break;
			case 13:
				i_type = "addic.";
				arg_r1();
				arg_r2();
				arg_SIMM();
				break;
			case 14:
				i_type = "addi";
				arg_r1();
				arg_r2();
				arg_SIMM();
				break;
			case 15:
				i_type = "addis";
				arg_r1();
				arg_r2();
				arg_SIMM();
				break;
			case 16:
				i_type = "bc";
				if (LK) {
					i_type += "l";
				}
				BD = BD << 2;
				if (AA) {
					i_type += "a";
				} else {
					BD += position;
				}
				arg_BO();
				arg_BI();
				arg_BD();
				break;
			case 17:
				i_type = "sc";
				add_arg();
				break;
			case 18:
				i_type = "b";
				if (LK) {
					i_type += "l";
				}
				LI = LI << 2;
				if (AA) {
					i_type += "a";
				} else {
					LI += position;
				}
				stream << itoh(LI);
				add_arg();
				break;
			case 19:
				switch (e_decider) {
				case 0:
					i_type = "mcrf";
					arg_crf1();
					arg_crf2();
					break;
				case 16:
					i_type = "bclr";
					if (r1 == 20) {
						i_type = "blr";
						add_arg();
						func_end = true;
					} else {
						arg_BO();
						arg_BI();
					}
					if (LK) {
						i_type += "l";
					}
					break;
				case 50:
					i_type = "rfi";
					add_arg();
					break;
				case 150:
					i_type = "isync";
					add_arg();
					break;
				case 193:
					i_type = "crxor";
					if (r1 == r2 && r2 == r3) {
						i_type = "crclr";
						arg_crb1();
					} else {
						arg_crb1();
						arg_crb2();
						arg_crb3();
					}
					break;
				case 257:
					i_type = "crand";
					arg_crb1();
					arg_crb2();
					arg_crb3();
					break;
				case 289:
					i_type = "creqv";
					if (r1 == r2 && r2 == r3) {
						i_type = "crset";
						arg_crb1();
					} else {
						arg_crb1();
						arg_crb2();
						arg_crb3();
					}
					break;
				case 449:
					i_type = "cror";
					if (r2 == r3) {
						i_type = "crmove";
						arg_crb1();
						arg_crb2();
					} else {
						arg_crb1();
						arg_crb2();
						arg_crb3();
					}
					break;
				case 528:
					i_type = "bcctr";
					if (r1 == 20) {
						i_type = "bctr";
						add_arg();
					} else {
						arg_BO();
						arg_BI();
					}
					if (LK) {
						i_type += "l";
					}
					break;
				}
				break;
			case 20:
				i_type = "rlwimi";
				if (RC) {
					i_type += ".";
				}
				arg_r2();
				arg_r1();
				arg_3();
				arg_4();
				arg_5();
				break;
			case 21:
				i_type = "rlwinm";
				if (RC) {
					i_type += ".";
				}
				arg_r2();
				arg_r1();
				arg_3();
				arg_4();
				arg_5();
				break;
			case 23:
				i_type = "rlwnm-rlwnm.";
				if (RC) {
					i_type += ".";
				}
				arg_r2();
				arg_r1();
				arg_r3();
				arg_4();
				arg_5();
				break;
			case 24:
				i_type = "ori";
				if (r2 == 0 && r1 == 0 && UIMM == 0) {
					i_type = "nop";
					add_arg();
				} else {
					arg_r2();
					arg_r1();
					arg_UIMM();
				}
				break;
			case 25:
				i_type = "oris";
				arg_r2();
				arg_r1();
				arg_UIMM();
				break;
			case 26:
				i_type = "xori";
				arg_r2();
				arg_r1();
				arg_UIMM();
				break;
			case 27:
				i_type = "xoris";
				arg_r2();
				arg_r1();
				arg_UIMM();
				break;
			case 28:
				i_type = "andi.";
				arg_r2();
				arg_r1();
				arg_UIMM();
				break;
			case 29:
				i_type = "andis.";
				arg_r2();
				arg_r1();
				arg_UIMM();
				break;
			case 31:
				switch (decider) {
				case 8:
					i_type = "subfc";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 10:
					i_type = "addc";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 11:
					i_type = "mulhwu";
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 23:
					i_type = "lwzx";
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 26:
					i_type = "cntlzw";
					if (RC) {
						i_type += ".";
					}
					arg_r2();
					arg_r1();
					break;
				case 40:
					i_type = "subf";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 75:
					i_type = "mulhw";
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 104:
					i_type = "neg";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					break;
				case 136:
					i_type = "subfe";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 138:
					i_type = "adde";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 200:
					i_type = "subfze";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					break;
				case 202:
					i_type = "addze";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					break;
				case 215:
					i_type = "stbx";
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 234:
					i_type = "addme";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					break;
				case 235:
					i_type = "mullw";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 266:
					i_type = "add";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 339:
					i_type = "mfspr";
					arg_r1();
					d = get_range(12, 16) + (get_range(17, 21) << 5);
					if (d == 1) {
						i_type = "mfxer";
					} else if (d == 8) {
						i_type = "mflr";
					} else if (d == 9) {
						i_type = "mfctr";
					} else {
						arg_d();
					}
					break;
				case 444:
					i_type = "or";
					if (r1 == r3) {
						i_type = "mr";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r2();
					arg_r1();
					if (r1 != r3) {
						arg_r3();
					}
					break;
				case 459:
					i_type = "divwu";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				case 467:
					i_type = "mtspr";
					arg_r1();
					d = get_range(12, 16) + (get_range(17, 21) << 5);
					if (d == 1) {
						i_type = "mtxer";
					} else if (d == 8) {
						i_type = "mtlr";
					} else if (d == 9) {
						i_type = "mtctr";
					} else {
						arg_d();
					}
					break;
				case 491:
					i_type = "divw";
					if (OE) {
						i_type += "o";
					}
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r2();
					arg_r3();
					break;
				default:
					switch (e_decider) {
					case 0:
						if (get_bit(11)) {
							i_type = "cmp";
						}
						else {
							i_type = "cmpw";
						}
						arg_crf1();
						if (get_bit(11)) {
							stream << "1";
							add_arg();
						}
						arg_r2();
						arg_r3();
						break;
					case 19:
						i_type = "mfcr";
						arg_r1();
						break;
					case 24:
						i_type = "slw";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 28:
						i_type = "and";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 32:
						if (get_bit(11)) {
							i_type = "cmpl";
						}
						else {
							i_type = "cmplw";
						}
						arg_crf1();
						if (get_bit(11)) {
							stream << "1";
							add_arg();
						}
						arg_r2();
						arg_r3();
						break;
					case 54:
						i_type = "dcbst";
						arg_r2();
						arg_r3();
						break;
					case 60:
						i_type = "andc";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 83:
						i_type = "mfmsr";
						arg_r1();
						break;
					case 86:
						i_type = "dcbf";
						arg_r2();
						arg_r3();
						break;
					case 87:
						i_type = "lbzx";
						arg_r1();
						arg_r2();
						arg_r3();
						break;
					case 124:
						i_type = "nor";
						if (r1 == r3) {
							i_type = "not";
						}
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						if (r1 != r3) {
							arg_r3();
						}
						break;
					case 144:
						i_type = "mtcrf";
						if (crm == 0xFF) {
							i_type = "mtcr";
							arg_r1();
						} else {
							stream << crm;
							add_arg();
							arg_r1();
						}
						break;
					case 146:
						i_type = "mtmsr";
						arg_r1();
						break;
					case 151:
						i_type = "stwx";
						arg_r1();
						arg_r2();
						arg_r3();
						break;
					case 210:
						i_type = "mtsr";
						arg_SR();
						arg_r1();
						break;
					case 278:
						i_type = "dcbt";
						arg_r2();
						arg_r3();
						break;
					case 279:
						i_type = "lhzx";
						arg_r1();
						arg_r2();
						arg_r3();
						break;
					case 284:
						i_type = "eqv";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 316:
						i_type = "xor";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 343:
						i_type = "lhax";
						arg_r1();
						arg_r2();
						arg_r3();
						break;
					case 371:
						i_type = "mftb";
						arg_r1();
						d = get_range(12, 16) + (get_range(17, 21) << 5);
						if (d == 269) {
							i_type = "mftbu";
						} else if (d != 268) {
							arg_d();
						}
						break;
					case 407:
						i_type = "sthx";
						arg_r1();
						arg_r2();
						arg_r3();
						break;
					case 412:
						i_type = "orc";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 470:
						i_type = "dcbi";
						arg_r2();
						arg_r3();
						break;
					case 512:
						i_type = "mcrxr";
						arg_crf1();
						break;
					case 534:
						i_type = "lwbrx";
						arg_r1();
						arg_r2();
						arg_r3();
						break;
					case 536:
						i_type = "srw";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 595:
						i_type = "mfsr";
						arg_r1();
						arg_SR();
						break;
					case 598:
						i_type = "sync";
						add_arg();
						break;
					case 599:
						i_type = "lfdx";
						arg_fr1();
						arg_r2();
						arg_r3();
						break;
					case 662:
						i_type = "stwbrx";
						arg_r1();
						arg_r2();
						arg_r3();
						break;
					case 663:
						i_type = "stfsx";
						arg_fr1();
						arg_r2();
						arg_r3();
						break;
					case 792:
						i_type = "sraw";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_r3();
						break;
					case 824:
						i_type = "srawi";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						arg_3();
						break;
					case 922:
						i_type = "extsh";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						break;
					case 954:
						i_type = "extsb";
						if (RC) {
							i_type += ".";
						}
						arg_r2();
						arg_r1();
						break;
					case 982:
						i_type = "icbi";
						arg_r2();
						arg_r3();
						break;
					case 983:
						i_type = "stfiwx";
						arg_fr1();
						arg_r2();
						arg_r3();
						break;
					}
					break;
				}
				break;
			case 32:
				i_type = "lwz";
				arg_r1();
				arg_dr2();
				break;
			case 33:
				i_type = "lwzu";
				arg_r1();
				arg_dr2();
				break;
			case 34:
				i_type = "lbz";
				arg_r1();
				arg_dr2();
				break;
			case 35:
				i_type = "lbzu";
				arg_r1();
				arg_dr2();
				break;
			case 36:
				i_type = "stw";
				arg_r1();
				arg_dr2();
				break;
			case 37:
				i_type = "stwu";
				arg_r1();
				arg_SIMMr2();
				break;
			case 38:
				i_type = "stb";
				arg_r1();
				arg_dr2();
				break;
			case 39:
				i_type = "stbu";
				arg_r1();
				arg_dr2();
				break;
			case 40:
				i_type = "lhz";
				arg_r1();
				arg_SIMMr2();
				break;
			case 41:
				i_type = "lhzu";
				arg_r1();
				arg_dr2();
				break;
			case 42:
				i_type = "lha";
				arg_r1();
				arg_dr2();
				break;
			case 43:
				i_type = "lhau";
				break;
			case 44:
				i_type = "sth";
				arg_r1();
				arg_dr2();
				break;
			case 45:
				i_type = "sthu";
				arg_r1();
				arg_dr2();
				break;
			case 46:
				i_type = "lmw";
				arg_r1();
				arg_dr2();
				break;
			case 47:
				i_type = "stmw";
				arg_r1();
				arg_dr2();
				break;
			case 48:
				i_type = "lfs";
				arg_fr1();
				arg_dr2();
				break;
			case 49:
				i_type = "lfsu";
				arg_r1();
				arg_dr2();
				break;
			case 50:
				i_type = "lfd";
				arg_fr1();
				arg_SIMMr2();
				break;
			case 51:
				i_type = "lfdu";
				arg_r1();
				arg_dr2();
				break;
			case 52:
				i_type = "stfs";
				arg_fr1();
				arg_dr2();
				break;
			case 53:
				i_type = "stfsu";
				arg_r1();
				arg_dr2();
				break;
			case 54:
				i_type = "stfd";
				arg_fr1();
				arg_dr2();
				break;
			case 55:
				i_type = "stfdu";
				arg_r1();
				arg_dr2();
				break;
			case 56:
				i_type = "lfq";
				arg_fr1();
				arg_dr2();
				break;
			case 59:
				switch (r5) {
				case 18:
					i_type = "fdivs";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr3();
					break;
				case 20:
					i_type = "fsubs";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr3();
					break;
				case 21:
					i_type = "fadds";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr3();
					break;
				case 24:
					i_type = "fres";
					if (RC) {
						i_type += ".";
					}
					arg_r1();
					arg_r3();
					break;
				case 25:
					i_type = "fmuls";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					break;
				case 28:
					i_type = "fmsubs";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				case 29:
					i_type = "fmadds";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				case 30:
					i_type = "fnmsubs";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				case 31:
					i_type = "fnmadds";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				}
				break;
			case 60:
				i_type = "stfq";
				arg_fr1();
				arg_dr2();
				break;
			case 63:
				switch (r5) {
				case 18:
					i_type = "fdiv";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr3();
					break;
				case 20:
					i_type = "fsub";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr3();
					break;
				case 21:
					i_type = "fadd";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr3();
					break;
				case 25:
					i_type = "fmul";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					break;
				case 26:
					i_type = "frsqrte";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr3();
					break;
				case 28:
					i_type = "fmsub";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				case 29:
					i_type = "fmadd";
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				case 30:
					i_type = "fnmsub";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				case 31:
					i_type = "fnmadd";
					if (RC) {
						i_type += ".";
					}
					arg_fr1();
					arg_fr2();
					arg_fr4();
					arg_fr3();
					break;
				default:
					switch (e_decider) {
					case 0:
						i_type = "fcmpu";
						arg_crf1();
						arg_fr2();
						arg_fr3();
						break;
					case 12:
						i_type = "frsp";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						arg_fr3();
						break;
					case 14:
						i_type = "fctiw";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						arg_fr3();
						break;
					case 15:
						i_type = "fctiwz";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						arg_fr3();
						break;
					case 32:
						i_type = "fcmpo";
						arg_crf1();
						arg_fr2();
						arg_fr3();
						break;
					case 38:
						i_type = "mtfsb1";
						if (RC) {
							i_type += ".";
						}
						arg_crb1();
						break;
					case 40:
						i_type = "fneg";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						arg_fr3();
						break;
					case 64:
						i_type = "mcrfs";
						arg_crf1();
						arg_crf2();
						break;
					case 70:
						i_type = "mtfsb0";
						if (RC) {
							i_type += ".";
						}
						arg_crb1();
						break;
					case 72:
						i_type = "fmr";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						arg_fr3();
						break;
					case 136:
						i_type = "fnabs";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						arg_fr3();
						break;
					case 264:
						i_type = "fabs";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						arg_fr3();
						break;
					case 583:
						i_type = "mffs";
						if (RC) {
							i_type += ".";
						}
						arg_fr1();
						break;
					case 711:
						i_type = "mtfsf";
						if (RC) {
							i_type += ".";
						}
						stream << get_range(8, 15);
						add_arg();
						arg_fr3();
						break;
					}
					break;
				}
				break;
			default:
				i_type = "BAD OPCODE";
				stream << (int)opcode;
				add_arg();
				break;
			}

			if (func_end && i_type != "blr") {
				output << "; function: f_" << std::hex << position << std::dec << " at " << itoh(position) << endl;
				func_end = false;
			}

			string hex = itoh(position);
			string padding(hex_length - hex.length(), ' ');
			output << padding << hex << "    " << ctoh(instruction[0]) << " " << ctoh(instruction[1]) << " " <<
				ctoh(instruction[2]) << " " << ctoh(instruction[3]) << "    ";

			output << i_type;
			for (std::vector<string>::iterator arg = args.begin(); arg != args.end(); arg++) {
				if (arg != args.begin()) {
					output << ",";
				}
				output << " " << *arg;
			}
			if (args.size() == 0) {
				output << " FIX ME";
			}
			output << endl;
		}
		std::cout << "PPC disassembly finished" << endl;
	}

	void read_data(string file_in, string file_out, int start, int end) {
		std::cout << "Reading data section" << endl;
		std::fstream input(file_in, ios::in | ios::binary);
		std::fstream output(file_out, ios::out);

		//TODO: Data guessing (tm)
	}

	void read_data(REL *to_read, Section *section, std::vector<REL*> knowns, string file_out) {
		// open the file, look through every import in every REL file to check if they refer to a
		// location in the right area in this one. If they do, add it to the list. Once done, go through
		// the list and generate output values. Anything leftover goes in a separate section clearly marked.
		std::cout << "Reading REL data section" << endl;
		std::fstream output(file_out, ios::out);
		std::set<int> offsets;
		for (std::vector<REL*>::iterator rel_r = knowns.begin(); rel_r != knowns.end(); rel_r++) {
			REL *rel = (*rel_r);
			for (std::vector<Import>::iterator imp = rel->imports.begin(); imp != rel->imports.end(); imp++) {
				if (imp->module != to_read->id && rel->id != to_read->id) {
					continue;
				}
				for (std::vector<Relocation>::iterator reloc = imp->instructions.begin(); reloc != imp->instructions.end(); reloc++) {
					if (reloc->get_src_section().id == section->id && imp->module == rel->id) {
						offsets.insert(reloc->get_src_offset());
					}
					if (rel->id == to_read->id && reloc->get_dest_section().id == section->id) {
						offsets.insert(reloc->get_dest_offset());
					}
				}
			}
		}
		for (int offset : offsets) {
			output << itoh(offset) << ": ";
			output << ctoh(section->data[offset++ - section->offset]) << endl;
			// Need to add non ASCII stuff later
			char dat = section->data[offset - section->offset];
			string out = "";
			while (dat >= 32 && dat <= 126 && offsets.find(offset) == offsets.end()) {
				out.append({ dat });
				dat = section->data[++offset - section->offset];
			}
			if (dat == 0) {
				std::cout << out << endl;
			} else {
				
			}
		}
		std::cout << "REL data section read complete" << endl;
	}

}