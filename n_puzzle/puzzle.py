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

	def map_from_file(self, file):
		data = open(file)
		readed = []
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
		if len(readed) != self.size:
			raise Exception("The map must be of size {}x{}".format(self.size, self.size))
		
		return readed

	def random_map(self):
		all_num = list(range(self.size ** 2))
		generated = np.zeros((self.size, self.size))
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

Puzzle(use_file=True, input_file="test_map")