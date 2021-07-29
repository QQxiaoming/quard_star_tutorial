(:*******************************************************:)
(: Test: K2-preceding-siblingAxis-4                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:count() to the result of axis preceding. :)
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
count(root($i)//leaf/preceding::node())