declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-11 :)
(: description : Evaluation of a validate expression that a newly lax validated node does not have a parent (read from a file).:)
(: Use fn:count to avoid empty file. :)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

let $var := validate lax {exactly-one($input-context1/DataValues/Strings)}
return
  fn:count($var/parent::node())
