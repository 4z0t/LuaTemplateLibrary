function Test()

    local t = MakeArray(10)
    print(#t)
    for i = 1, #t do
        print(t[i])
    end
    print(DoubleInt(2, 3))
    print(TripleInt(2, 3, 5))

    local tc = TestClass.new()
    tc:print()
    tc:aboba()
    PrintInc()
    PrintInc()
    PrintInc()
    print(VectorLen{1,2,3})
    local v = VectorSum({1,2,3},{4,5,6})
    print(table.unpack(v))
    print(GetSystemTime())
    SayHello()
    SayBye()
    SayFoo()
    Say(1)
    print(Gamma(0.5))
    print(MyFunc(2, 5))
end


local function MySum( v1, v2)
	return {v1[1]+v2[1],v1[2]+v2[2],v1[3]+v2[3]}
end


function TestSum(n)
    local VectorSum = VectorSum
    local v1 = {1,2,3}
    local v2 = {4,5,6}
    local start_t = GetSystemTime()
    for i = 1,n do
    VectorSum(v1,v2)
    end

    return  GetSystemTime() - start_t 
end

function TestMySum(n)
    local VectorSum = MySum
    local v1 = {1,2,3}
    local v2 = {4,5,6}
    local start_t = GetSystemTime()
    for i = 1,n do
    VectorSum(v1,v2)
    end

    return  GetSystemTime() - start_t 
end


function Main( )
   Test()
   local n = 10000000
   print(TestSum(n))
   print(TestMySum(n))
end

