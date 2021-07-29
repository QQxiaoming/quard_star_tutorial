(: Name: NodeTesthc-1 :)
(: Description: Simple test for text type involving a boolean expression (and fn:true) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
  {$input-context1//text() and fn:true()}
</result>
