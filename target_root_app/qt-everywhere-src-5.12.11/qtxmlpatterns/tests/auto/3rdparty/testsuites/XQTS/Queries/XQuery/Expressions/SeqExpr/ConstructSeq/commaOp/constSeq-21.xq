(: Name: constSeq-21:)
(: Description: Constructing sequences.  Constructing a sequence, where both of its members contains invocation to "xs:dateTime()".:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

(xs:dateTime("1999-11-28T09:00:00Z"),xs:dateTime("1998-11-28T09:00:00Z"))