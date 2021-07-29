(: Name: preceding-23 :)
(: Description: Evaluation of the preceding axis from the last node in the document, :)
(: containing only attributes and elements. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
{
    (doc($input-context1)//node())[last()]/preceding::node(),
    empty((doc($input-context1)//node())[last()]/preceding::node())
}
</result>
