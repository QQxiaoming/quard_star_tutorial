(: Name: fn-root-17:)
(: Description: Evaluation of the fn:root function with argument set to computed text node by setting argument vias a variable.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

let $var := text {"a text Node"}
return
 fn:root(text {"A text Node"})