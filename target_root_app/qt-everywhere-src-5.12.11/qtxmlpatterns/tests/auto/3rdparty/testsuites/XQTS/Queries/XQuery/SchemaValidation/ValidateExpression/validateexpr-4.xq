declare default element namespace "http://www.w3.org/XQueryTestOrderBy";

(: name : validateexpr-4 :)
(: description : Evaluation of a validate expression that results on a comment node with imnplicit strict mode.:)

(: insert-start :)
import schema "http://www.w3.org/XQueryTestOrderBy";
declare variable $input-context1 external;
(: insert-end :)

validate {<!-- A Comment Node -->}