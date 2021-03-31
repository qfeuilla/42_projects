import numpy as np

class Puzzle():
	def __init__(self, use_file=False, size=0, input_file=None):
		self.size = size
		if use_file:
			if isinstance(input_file, str):
				self.current_map = self.map_from_file(input_file)
			else:
				raise Exception("Input file isnt a valid file")
		else:
			self.current_map = self.random_map()
		self.solution_maps = []
		print("map to solve : ")
		self.show_map(self.current_map)
		self.answer = self.generate_answer()

	def map_from_file(self, file):
		data = open(file)
		readed = []
		all_nb = []
		for i, line in enumerate(data.readlines()):
			line = line.replace("#", " # ")
			line = line.split()
			if line[0][0] == "#":
				continue
			if self.size == 0 and len(line) > 1 and line[1][0] != "#":
				raise Exception("first readable line should be the size of the puzzle")
			if self.size == 0:
				self.size = int(line[0])
				if (self.size < 3):
					raise Exception("puzzle size should be greater than 2")
				continue
			try:
				sp_index = line.index("#")
			except:
				sp_index = -1
			try:
				int_data = list(map(int,(line[:sp_index] if sp_index != -1 else line)))
			except:
				raise Exception("you forgot an uncommented string on line {}".format(i + 1))
			if (len(int_data) != self.size):
				raise Exception("each lines should have {} values".format(self.size))
			readed.append(int_data)
			all_nb += int_data
		if len(readed) != self.size:
			raise Exception("The map must be of size {}x{}".format(self.size, self.size))
		max_val = self.size ** 2
		for i in range(max_val):
			if all_nb.count(i) != 1:
				raise Exception("The map should contain one value of each number in the range [0, {}[".format(max_val))
		return np.array(readed, dtype=np.int)

	def random_map(self):
		all_num = list(range(self.size ** 2))
		generated = np.zeros((self.size, self.size), dtype=np.int)
		for y in range(self.size):
			for x in range(self.size):
				choice = np.random.choice(all_num)
				all_num.remove(choice)
				generated[y][x] = choice
		return generated

	def show_map(self, data=None):
		if data is None:
			data = self.current_map
		print('\n'.join([' '.join([str(int(l)).center(5, ' ') for l in lst]) for lst in data]))
	
	def inversion_num(self):
		ordered_map = list()
		x = 0
		y = 0

		for i in range(self.size * 2 - 1):
			res = i % 4
			if i > 0:
				x += (1 if res == 0 else -1) if not res % 2 else 0
				y += (1 if res == 1 else -1) if res % 2 else 0
			while ((x < self.size and res == 0) 
					or (y < self.size and res == 1) 
					or (x >= 0 and res == 2) 
					or (y >= 0 and res == 3)) and self.current_map[y][x] not in ordered_map:
				ordered_map.append(self.current_map[y][x])
				x += (1 if res == 0 else -1) if not res % 2 else 0
				y += (1 if res == 1 else -1) if res % 2 else 0
			x += (-1 if res == 0 else 1) if not res % 2 else 0
			y += (-1 if res == 1 else 1) if res % 2 else 0
		reverse_global = 0
		map_index = {}
		for i, k in enumerate(ordered_map):
			map_index[k] = i
		for k in map_index.keys():
			curr_ind = map_index[k]
			for smaller in range(1, k):
				if map_index[smaller] > curr_ind:
					reverse_global += 1
		return reverse_global

	def is_sovable(self):
		div = not self.inversion_num() % 2
		if self.size % 2 == 1:
			return div
		else:
			y, x = np.where(self.current_map == 0)
			if y % 2 == 0:
				return not div
			else:
				return div

	def generate_answer(self):
		answ = np.zeros((self.size, self.size), dtype=np.int)
		current = 1
		x = 0
		y = 0
		for i in range(self.size * 2 - 1):
			res = i % 4
			if i > 0:
				x += (1 if res == 0 else -1) if not res % 2 else 0
				y += (1 if res == 1 else -1) if res % 2 else 0
			while ((x < self.size and res == 0) 
					or (y < self.size and res == 1) 
					or (x >= 0 and res == 2) 
					or (y >= 0 and res == 3)) and answ[y][x] == 0:
				answ[y][x] = current
				current += 1
				x += (1 if res == 0 else -1) if not res % 2 else 0
				y += (1 if res == 1 else -1) if res % 2 else 0
			x += (-1 if res == 0 else 1) if not res % 2 else 0
			y += (-1 if res == 1 else 1) if res % 2 else 0
		answ[y][x] = 0
		return answ

puzzle = Puzzle(use_file=True, input_file="test_map")

print(puzzle.inversion_num())
print(puzzle.is_sovable())
print(puzzle.generate_answer())