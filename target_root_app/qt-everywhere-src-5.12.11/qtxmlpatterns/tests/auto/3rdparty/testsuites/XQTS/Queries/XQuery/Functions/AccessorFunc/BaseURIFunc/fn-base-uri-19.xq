(: Name: fn-base-uri-19 :)
(: Description: Evaluation of base-uri function with argument set to a directly constructed element node (via FLOWR expr). :)
(: Use the xml-base attribute :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := <anElement xml:base="http://www.examples.com">With some content</anElement>
return
   fn:string(fn:base-uri($var))