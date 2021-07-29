(:*******************************************************:)
(: Test: K2-followingAxis-3                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Evaluate the child node from root of a tree. :)
(:*******************************************************:)
declare variable $i := <root>
    <child/>
    <child/>
    <child>
        <child2>
            <child3>
                    <leaf/>
            </child3>
        </child2>
    </child>
</root>;
empty(root($i)/following::node())