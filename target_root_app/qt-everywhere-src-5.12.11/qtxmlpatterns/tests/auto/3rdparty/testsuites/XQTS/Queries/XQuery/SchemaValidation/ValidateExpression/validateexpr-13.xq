declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-13 :)
(: description : Evaluation of a validate expression that a newly created lax validated node does not have a parent (read from a file).:)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

let $var := validate lax {<Strings><orderData>A String</orderData></Strings>}
return
  fn:count($var/parent::node())