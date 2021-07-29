(: Name: fn-idref-4 :)
(: Description: Evaluation of fn:idref with node not being from document where root is the document element. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement>Some Content</anElement>
return
  fn:idref("argument1", $var)