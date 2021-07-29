(: Name: preceding-22 :)
(: Description: Evaluation of the preceding axis from the last node in the document. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
{
    (doc($input-context1)//node())[last()]/preceding::node()
}
</result>
