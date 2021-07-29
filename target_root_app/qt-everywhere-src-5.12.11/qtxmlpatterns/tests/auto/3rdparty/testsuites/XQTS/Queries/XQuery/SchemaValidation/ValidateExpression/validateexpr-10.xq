declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-10 :)
(: description : Evaluation of a validate expression that a newly strict validated node does not have a parent (read from a file).:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

let $var := validate strict {exactly-one($input-context1/DataValues/Strings)}
return
  fn:count($var/parent::node())
