(:*******************************************************:)
(: Test: K2-following-siblingAxis-1                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply following-sibling to a child whose siblings are the last in a document. :)
(:*******************************************************:)
<root>
    <child/>
    <child/>
    <child/>
</root>/child[1]/following-sibling::node()