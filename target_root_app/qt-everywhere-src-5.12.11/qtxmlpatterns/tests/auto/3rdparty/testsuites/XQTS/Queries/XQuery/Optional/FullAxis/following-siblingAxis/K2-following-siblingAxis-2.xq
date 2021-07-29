(:*******************************************************:)
(: Test: K2-following-siblingAxis-2                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply following-sibling to a child whose last nodes in document order are attributes. :)
(:*******************************************************:)
<root>
    <child/>
    <child/>
    <child attr="foo" attr2="foo"/>
</root>/child[1]/following-sibling::node()