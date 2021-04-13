import numpy as np
import heapq
from tqdm import tqdm
import sys

class Puzzle_Node():
	def __init__(self, parent=None, node_map=None, size=0, previous="None"):
		self.parent = parent
		self.previous = previous
		self.node_map = node_map
		self.size = size
		self._hash = hash(str(node_map))
		self.g = 0
		self.h = 0

	def __eq__(self, other):
		return np.sum(np.equal(self.node_map, other.node_map)) == self.size**2
	
	def __lt__(self, other):
		if self.h != other.h:
			return self.h < other.h
		return self.g < other.g

	def __hash__(self):
		return self._hash

def is_in(child_list, child, use_sup=False):
	for el_child in child_list:
		if child == el_child and (child.g > el_child.g or not use_sup) :
			return True
	return False

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
		print("map to solve : ")
		self.show_map(self.current_map)
		self.solution = self.generate_solution()
		self.pbar = None

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
	
	def unfold_map(self, data=None):
		if data is None:
			data = self.current_map
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
					or (y >= 0 and res == 3)) and data[y][x] not in ordered_map:
				ordered_map.append(data[y][x])
				x += (1 if res == 0 else -1) if not res % 2 else 0
				y += (1 if res == 1 else -1) if res % 2 else 0
			x += (-1 if res == 0 else 1) if not res % 2 else 0
			y += (-1 if res == 1 else 1) if res % 2 else 0
		return ordered_map

	def inversion_num(self, data=None):
		if data is None:
			data = self.current_map
		ordered_map = self.unfold_map(data)
		reverse_global = 0
		map_index = {k:i for i, k in enumerate(ordered_map)}
		for k in map_index.keys():
			curr_ind = map_index[k]
			for smaller in range(1, k):
				if map_index[smaller] > curr_ind:
					reverse_global += 1
		return reverse_global

	def hamming(self, data=None):
		if data is None:
			data = self.current_map
		return np.sum(np.not_equal(data, self.solution))

	def is_sovable(self):
		return not self.inversion_num() % 2

	def generate_solution(self):
		sol = np.zeros((self.size, self.size), dtype=np.int)
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
					or (y >= 0 and res == 3)) and sol[y][x] == 0:
				sol[y][x] = current
				current += 1
				x += (1 if res == 0 else -1) if not res % 2 else 0
				y += (1 if res == 1 else -1) if res % 2 else 0
			x += (-1 if res == 0 else 1) if not res % 2 else 0
			y += (-1 if res == 1 else 1) if res % 2 else 0
		sol[y][x] = 0
		return sol

	def manhattan_loc(self, x, y, data):
		sy, sx = np.where(self.solution == data[y][x])
		return np.absolute(x - sx) + np.absolute(y - sy)

	def euclidian_loc(self, x, y, data):
		sy, sx = np.where(self.solution == data[y][x])
		return np.sqrt(np.square(x - sx) + np.square(y - sy))

	def distance(self, _type="manhattan"):
		if (_type == "manhattan"):
			dist_func = self.manhattan_loc
		else:
			dist_func = self.euclidian_loc
		def dist(data=None):
			if data is None:
				data = self.current_map
			man_map = np.zeros((self.size, self.size), dtype=np.int)
			for y in range(self.size):
				for x in range(self.size):
					man_map[y][x] = dist_func(x, y, data)
			return np.sum(man_map)
		return dist

	def solve(self, metric="manhattan"):
		if metric == "hamming":
			metric_fnc = self.hamming
		else:
			metric_fnc = self.distance(_type=metric)
		
		# Create start and end node
		start_metric = metric_fnc(self.current_map)
		start_node = Puzzle_Node(None, self.current_map, self.size)
		start_node.g = 0
		start_node.h = start_metric
		end_node = Puzzle_Node(None, self.solution, self.size)
		end_node.g = end_node.h = 0

		# Initialize both open and closed list
		open_list = []
		heapq.heappush(open_list, start_node)
		closed_set = {}
		open_set = {}
		open_set[start_node] = start_node

		min_manhattan = 10000
		# Loop until you find the end
		while len(open_list):
			while True:
				try:
					current_node = heapq.heappop(open_list)
				except:
					current_node = None
					break
				#skip if the node has been deleted from the set
				if current_node.previous == "Delete":
					continue
				del open_set[current_node]
				closed_set[current_node] = 0
				break
			
			if current_node is None:
				print("No solution")
				return 0

			# Found the goal
			if current_node == end_node:
				path = []
				current = current_node
				while current is not None:
					path.append(current.node_map)
					current = current.parent
				return (path[::-1], len(open_set), len(closed_set)) # Return reversed path

			# Generate children
			children = []
			y, x = np.where(current_node.node_map == 0)
			y = y[0]
			x = x[0]
			for a in [-1, 1]:
				newx = x + a
				if newx >= 0 and newx < self.size:
					new_map = current_node.node_map.copy()
					new_map[y][x] = new_map[y][newx]
					new_map[y][newx] = 0
					children.append(Puzzle_Node(current_node, new_map, self.size))
			for b in [-1, 1]:
				newy = y + b
				if newy >= 0 and newy < self.size:
					new_map = current_node.node_map.copy()
					new_map[y][x] = new_map[newy][x]
					new_map[newy][x] = 0
					children.append(Puzzle_Node(current_node, new_map, self.size))

			for child in children:
				child.g = current_node.g + 1
				child.h = metric_fnc(child.node_map)
				if child.h < min_manhattan:
					if self.pbar is None:
						self.pbar = tqdm(total=child.h, file=sys.stdout)
					else:
						self.pbar.update(min_manhattan - child.h)
					min_manhattan = child.h

				if child in closed_set:
					continue
				elif child in open_set:
					old = open_set[child]
					if child < old:
						old.previous = "Delete"
						heapq.heappush(open_list, child)
						open_set[child] = child
				else:
					heapq.heappush(open_list, child)
					open_set[child] = child

puzzle = Puzzle(size=6)

import time

if puzzle.is_sovable():
	sol = puzzle.solve(metric="manhattan")
	print("space complexity: {}\ntime complexity: {}".format(sol[1], sol[2]))
	for node in sol[0]:
		print(node)
else:
	print("puzzle not solvable")