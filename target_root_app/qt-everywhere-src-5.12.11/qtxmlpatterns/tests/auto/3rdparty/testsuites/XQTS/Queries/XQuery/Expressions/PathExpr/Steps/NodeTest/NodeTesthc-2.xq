(: Name: NodeTesthc-2 :)
(: Description: Simple test for text type involving a boolean expression (or fn:true) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
  {$input-context1//text() or fn:true()}
</result>
