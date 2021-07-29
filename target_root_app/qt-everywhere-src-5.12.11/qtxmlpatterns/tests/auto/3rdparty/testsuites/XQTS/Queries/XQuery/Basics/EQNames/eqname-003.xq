xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-003                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use EQName in a function name                 :)
(:*******************************************************:)

declare function "http://www.example.com/ns/my":fn ($a as xs:integer) as xs:integer {
   $a + 2
};

<out>{"http://www.example.com/ns/my":fn(12)}</out>