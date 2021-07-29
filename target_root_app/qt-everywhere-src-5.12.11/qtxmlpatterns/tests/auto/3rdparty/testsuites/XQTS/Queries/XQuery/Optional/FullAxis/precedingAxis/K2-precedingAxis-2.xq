(:*******************************************************:)
(: Test: K2-precedingAxis-2                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Evaluate from a node that has no preceding nodes. :)
(:*******************************************************:)
declare variable $i := <root>
    <child>
        <child2>
            <child3>
                    <leaf/>
            </child3>
            <following/>
        </child2>
        <following/>
    </child>
    <following/>
</root>;
empty(root($i)//leaf/preceding::node())