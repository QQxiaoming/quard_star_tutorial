declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-23 :)
(: description : Evaluation of a validate expression, whose operand node is a newly cosntructed top element :)
(: node for whom there is no declaration.:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

            
let $var := validate strict { element DataValues1 {element Strings {"data"}}}
return $var