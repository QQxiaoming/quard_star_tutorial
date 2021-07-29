declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : combined-errors-3 :)
(: description : Evaluates simple Schema Validation feature to generate error code if feature not supported.:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

validate strict {<Strings><orderData>one string</orderData></Strings>}