xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-001                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use EQName in a variable name                 :)
(:*******************************************************:)

declare variable $"http://www.example.com/ns/my":var := 12;

<out>{$"http://www.example.com/ns/my":var}</out>