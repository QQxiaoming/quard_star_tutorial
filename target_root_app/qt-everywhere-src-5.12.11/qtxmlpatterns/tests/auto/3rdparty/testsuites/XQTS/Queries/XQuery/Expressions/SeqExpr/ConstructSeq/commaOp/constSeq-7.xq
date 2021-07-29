(: Name: constSeq-7 :)
(: Description: Constructing sequences.  Constructing a sequence, where one of the members contains invocation to "fn:string-length" function.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(1, fn:string-length("AB"), 3, 4, 5)