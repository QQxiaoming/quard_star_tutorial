(:*******************************************************:)
(: Test: K2-preceding-siblingAxis-1                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Iterate from the root node.                  :)
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
empty(root($i)/preceding-sibling::node())