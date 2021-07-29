(:*******************************************************:)
(: Test: K2-following-siblingAxis-4                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Evaluate fn:count() on a range of nodes, navigated with axis following. :)
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
root($i)//count(following-sibling::node())