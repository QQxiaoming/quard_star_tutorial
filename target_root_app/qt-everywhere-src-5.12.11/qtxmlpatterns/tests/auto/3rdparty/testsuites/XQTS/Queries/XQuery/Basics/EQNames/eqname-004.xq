xquery version "1.1";
(:*******************************************************:)
(:Test: eqname-004                                       :)
(:Written By: Michael Kay                                :)
(:Date: 16 March 2010                                    :)
(:Purpose: Use EQName in a type name                     :)
(:*******************************************************:)

declare function local:fn ($a as "http://www.w3.org/2001/XMLSchema":integer) as element("http://www.example.com/ns/my":e) {
   <e xmlns="http://www.example.com/ns/my">{$a}</e>
};

<out>{local:fn(12)}</out>