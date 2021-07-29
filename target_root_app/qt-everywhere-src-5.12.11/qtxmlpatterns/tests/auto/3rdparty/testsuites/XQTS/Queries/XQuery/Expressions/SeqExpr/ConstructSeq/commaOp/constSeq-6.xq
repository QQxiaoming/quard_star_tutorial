(: Name: constSeq-6 :)
(: Description: Constructing sequences.  Constructing a sequence, where one of the members contains invocation to "fn:count" function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1, fn:count((1, 2)), 3, 4, 5)