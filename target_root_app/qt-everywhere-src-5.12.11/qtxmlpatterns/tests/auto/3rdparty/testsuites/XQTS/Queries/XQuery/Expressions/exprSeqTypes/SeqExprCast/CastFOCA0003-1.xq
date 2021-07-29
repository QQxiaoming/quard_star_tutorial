(: Name: CastFOCA0003:)
(: Description: casting an xs:integer with a value to big for that type.:)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

xs:integer(99e100)