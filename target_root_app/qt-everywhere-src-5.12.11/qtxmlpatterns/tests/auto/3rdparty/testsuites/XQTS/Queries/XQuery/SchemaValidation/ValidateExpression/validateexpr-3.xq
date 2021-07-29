declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-3 :)
(: description : Evaluation of a validate expression with newly constructed Element node and lax validation.:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

let $var := <Strings><orderData>one string</orderData></Strings>
return
validate lax {$var}