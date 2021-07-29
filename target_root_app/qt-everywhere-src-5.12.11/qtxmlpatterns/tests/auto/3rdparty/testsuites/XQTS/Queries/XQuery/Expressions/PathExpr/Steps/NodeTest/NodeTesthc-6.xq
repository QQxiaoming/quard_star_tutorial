(: Name: NodeTesthc-6 :)
(: Description: Simple test for Element node types involving a boolean expression (or fn:true) :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

<result>
  {$input-context1//overtime/node() or fn:true()}
</result>
