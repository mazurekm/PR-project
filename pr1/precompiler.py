from sys import argv

fd1=open(argv[1],'r')
fd2=open(argv[2],'r')

def generate_solver_file(data1,data2):
	data1=data1.split('\n')
	data1.remove('')
	for el in data1:
		tmp=el.split('=')
		data2=data2.replace(tmp[0],tmp[1])
	return data2

def main():
	fd1=open(argv[1],'r')
	fd2=open(argv[2],'r')
	fd3=open('out.pl','w')
	data1=fd1.read()
	data2=fd2.read()
	result=generate_solver_file(data1,data2)
	print(result)
	fd1.close()
	fd2.close()
if __name__=='__main__':
	main()
