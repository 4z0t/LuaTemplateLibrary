function Test()

    local t = MakeArray(10)
    print(#t)
    for i = 1, #t do
        print(t[i])
    end
    print(DoubleInt(2, 3))

    local tc = TestClass.new()
    tc:print()
    tc:aboba()
    PrintInc()
    PrintInc()
    PrintInc()

    SayHello()
    SayBye()
end

function Main( )
   Test()
end
