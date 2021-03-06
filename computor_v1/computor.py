
import sys

def sqrt(x):
	mid = 0
	lo = 0
	hi = x
	accuracy = 100
	while accuracy:
		mid = (lo + hi) / 2
		if (mid**2 == x):
			return mid
		if (mid**2 > x):
			hi = mid
		else:
			lo = mid
		accuracy -= 1
	return mid

try:
	inp = sys.argv[1]
	if (len(sys.argv) > 2):
		raise Exception()
except :
	print("please input a valid command string")
	exit()

valid = " X^0123456789-+=*"
count = 0
for v in valid:
	count += inp.count(v)
if count != len(inp):
	print("There is some invalid character in you equation")
	exit()

split_equ = inp.split('=')
if (len(split_equ) != 2 or "X" not in inp or len(split_equ[1].split()) == 0):
	print("input is not an equation with unknowns")
	exit()

def get_deg(term):
	if (term.count("^")):
		ind = term.index("^")
	elif (term.count("X")):
		return 1
	else:
		return 0
	try:
		int(term[ind+1]) # raise the error if not int on first
		end = ind+1
		while end < len(term) and (term[end].isnumeric()):
			end += 1
		return (int(term[ind+1:end]))
	except:
		print("please input a valid equation (no num after ^)")
		exit()

def simplify(term):
	'''
		This function take a term and symplified it suched that 
		X^0 is removed and remaining multiplications are simplfied
	'''
	if ("X^0" in term):
		term = term.replace("X^0", "1")
	nb = []
	Xpow = 0
	Xindices = [i for i, value in enumerate(term) if value == "X"]
	for i in Xindices:
		term = term[:i] + "*" + term[i:]
	for sub in term.split("*"):
		if sub != "":
			try:
				nb.append(float(sub))
			except: 
				Xpow += get_deg(sub)
	mul = 1
	for n in nb:
		mul *= n
	ret = "{}".format(mul) + ("*X{}".format("^" + str(Xpow) if Xpow > 1 else "") if Xpow != 0 else " ")
	if ret == "1*" or ret == "1 ":
		return None
	return ret.replace(" ", "")

def get_each_part(member):
	'''
		parse the member
	'''

	cmd = []
	for p in member.split("-"):
		for term in p.split("+"):
			s = simplify(term)
			if s:
				cmd.append(s)
			cmd.append("+")
		del cmd[-1]
		cmd.append("-")
	del cmd[-1]
	return cmd

left_split = get_each_part(split_equ[0])
righ_split = get_each_part(split_equ[1])

def reduce_p_m(part, inverse=False):
	'''
		reduce each occurence of - + to one and inverse
		for side switching
	'''
	last = "+"
	res = []
	for e in part:
		if e == "+":
			if last != "-":
				last = "+"
		elif e == "-": 
			if last == "-":
				last = "+"
			else:
				last ="-"
		else:
			if (inverse):
				if (last == "+"):
					res.append("-")
				else:
					res.append("+")
			else:
				res.append(last)
			last = "+"
			res.append(e)
	return res

left_member = reduce_p_m(left_split) + reduce_p_m(righ_split, inverse=True)

deg = {}
last_sign = "+"

# let's class each element by degree
for e in left_member:
	if e == "+" or e == "-":
		last_sign = e
	else:
		try:
			deg[get_deg(e)] += [last_sign + e]
		except:
			deg[get_deg(e)] = [last_sign + e]
		last_sign = "+"

# collapse each degree on one element
keys = list(deg.keys())
for k in keys:
	som = 0
	for e in deg[k]:
		som += float(e.split('*')[0])
	if som != 0:
		if (som == 1 and k != 0):
			deg[k] = ("X^{}".format(k), som) if k != 1 else ("X", som)
		elif (som == -1 and k != 0):
			deg[k] = ("-X^{}".format(k), som) if k != 1 else ("-X", som)
		else:
			deg[k] = (("{:g}X^{}".format(som, k), som) if k != 1 else ("{:g}X".format(som), som)) if k != 0 else ("{:g}".format(som), som)
	else:
		del deg[k]

reduced = ""

for it in deg.items():
	if it[1][0][0] == "-":
		reduced += "- " + it[1][0][1:] + " "
	else:
		reduced += ("+ " + it[1][0] + " ") if reduced != "" else it[1][0] + " "

reduced = reduced if reduced != "" else "0 "
print("Reduced form: " + reduced + "= 0")
pdeg = sorted(deg.keys(), key= lambda x:int(x))[-1] if len(deg.keys()) > 0 else 0
print("Polynomial degree: " + str(pdeg))
if (pdeg > 2):
	print("The polynomial degree is strictly greater than 2, I can't solve.")
elif (pdeg == 0):
	if reduced == "0 ":
		print("All real numbers are solutions")
	else:
		print("The equation has no solutions")
elif (pdeg == 1):
	a = deg[1][1]
	if not (0 in deg.keys()):
		b = 0
	else:
		b = deg[0][1]
	res = -(b / a)
	res = res if res != -0 else 0
	print("answer is -(b / a) with a = {} and b = {} :".format(a, b))
	print("ans = {:g}".format(res))
else:
	#let's solve the equation !!
	a = deg[2][1]
	if not (1 in deg.keys()):
		b = 0
	else:
		b = deg[1][1]
	if not (0 in deg.keys()):
		c = 0
	else:
		c = deg[0][1]
	delta = b**2 - 4 * a * c
	print("discriminant is calculated with :")
	print("delta = b^2 - 4 * a * c with a = {}, b = {} and c = {}".format(a, b, c))
	print("delta = {}".format(delta))
	tmp = -b / (2 * a)
	tmp2 = sqrt(delta if delta > 0 else -delta)
	if delta > 0:
		print("Discriminant is strictly positive, the two roots are:")
		print("root1 = (-b - sqrt(delta)) / (2 * a)")
		print("root1 = {:g}".format((-b - tmp2) / (2 * a)))
		print("root2 = (-b + sqrt(delta)) / (2 * a)")
		print("root2 = {:g}".format((-b + tmp2) / (2 * a)))
	if delta == 0:
		print("Discriminant is equal to zero, the root is:")
		print("root = -b / (2 * a)")
		print("root = {:g}".format(tmp))
	if delta < 0:
		print("Discriminant is strictly negative, the two roots are complex:")
		print("root1 = (-b - sqrt(-delta)) / (2 * a)")
		print("root1 = {:g}".format(tmp) + " - " + "{:g}".format(tmp2 / (2 * a)) + "i")
		print("root2 = (-b + sqrt(-delta)) / (2 * a)")
		print("root2 = {:g}".format(tmp) + " + " + "{:g}".format(tmp2 / (2 * a)) + "i")
