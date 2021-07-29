(:*******************************************************:)
(:Test:fn-normalize-space0args-1:)
(:Written By:Joanne Tong:)
(:Date:2005-09-23T15:54:03-04:00:)
(:Purpose:Test normalize-space without argument:)
(:*******************************************************:)

(: insert-start :)
declare variable $input-context external;
(: insert-end :)

$input-context//doc/normalize-space(zero-or-one(a[normalize-space() = 'Hello, How are you?']))