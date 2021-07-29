(:*******************************************************:)
(: Test: K2-precedingAxis-5                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply a numeric predicate to axis preceding, combined with fn:last(). The paranteses ensures the step is parsed as a primary expression, and hence is in document order, not reversed. :)
(:*******************************************************:)
declare variable $i := <root>
    <child>
        <preceding2/>
        <child2>
            <preceding1/>
            <child3>
                    <leaf/>
            </child3>
            <following/>
        </child2>
        <following/>
    </child>
    <following/>
</root>;
root($i)//leaf/(preceding::node())[last()]