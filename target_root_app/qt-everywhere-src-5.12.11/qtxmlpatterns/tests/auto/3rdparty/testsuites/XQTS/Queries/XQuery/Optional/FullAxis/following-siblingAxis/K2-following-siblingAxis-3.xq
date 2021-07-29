(:*******************************************************:)
(: Test: K2-following-siblingAxis-3                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:count() to the result of axis following-sibling. :)
(:*******************************************************:)
count(<root>
    <child/>
    <child/>
    <child attr="foo" attr2="foo"/>
</root>/child[1]/following-sibling::node())