(:*******************************************************:)
(: Test: K2-Serialization-12                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Write out elements and attributes that have the same names except for their prefixes. :)
(:*******************************************************:)
<e>
    <a:a xmlns:a="http://www.example.com/A" a:a="value"/>
    <b:a xmlns:b="http://www.example.com/A" b:a="value"/>
</e>