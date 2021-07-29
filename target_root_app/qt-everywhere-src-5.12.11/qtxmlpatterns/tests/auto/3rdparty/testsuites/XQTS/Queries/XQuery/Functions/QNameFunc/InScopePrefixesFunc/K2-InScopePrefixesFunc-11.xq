(:*******************************************************:)
(: Test: K2-InScopePrefixesFunc-11                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check the in-scope namespaces of different elements, constructed with direct constructors. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com/";
let $i := <e>
    <a xmlns=""/>
    <b xmlns="http://www.example.com/"/>
    <c xmlns="http://www.example.com/Second"/>
</e>
return (count(in-scope-prefixes($i)),
        count(in-scope-prefixes(exactly-one($i/*[namespace-uri() eq ""]))),
        count(in-scope-prefixes(exactly-one($i/b))),
        count(in-scope-prefixes(exactly-one($i/*[namespace-uri() eq "http://www.example.com/Second"]))),
        $i)