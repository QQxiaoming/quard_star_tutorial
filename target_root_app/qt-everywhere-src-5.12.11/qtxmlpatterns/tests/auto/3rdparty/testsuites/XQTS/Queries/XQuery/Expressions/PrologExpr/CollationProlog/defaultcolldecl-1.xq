(: Name: defaultcolldecl-1 :)
(: Description: Evaluation of a prolog with two default collation delarations.:)

declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint";
declare default collation "http://www.w3.org/2005/xpath-functions/collation/codepoint";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"