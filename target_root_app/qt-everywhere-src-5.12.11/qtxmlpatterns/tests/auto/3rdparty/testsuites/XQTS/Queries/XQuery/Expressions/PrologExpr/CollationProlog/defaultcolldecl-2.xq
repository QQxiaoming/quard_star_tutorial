(: Name: defaultcolldecl-2 :)
(: Description: Evaluation of a prolog with default collation delarations that should raise an error.:)

declare default collation "http://nonexistentcollition.org/ifsupportedwoooayouarethebestQueryimplementation/makeitharder";

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

"aaa"