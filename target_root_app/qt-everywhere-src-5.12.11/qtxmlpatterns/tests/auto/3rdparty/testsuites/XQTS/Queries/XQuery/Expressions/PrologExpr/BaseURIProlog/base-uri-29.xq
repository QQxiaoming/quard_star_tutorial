(: Name: base-uri-29 :)
(: Written by: Andrew Eisenberg :)
(: Description: relative base-uri through parent :)

declare base-uri "http://www.example.com/";

fn:base-uri(exactly-one((<elem xml:base="fluster/"><a xml:base="now"/></elem>)/a))
