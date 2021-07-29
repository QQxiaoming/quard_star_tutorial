(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-10                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Two attributes that have identical expanded names, but indifferent lexical names. :)
(:*******************************************************:)
declare namespace a = "http://www.example.com/";
<e xmlns:b="http://www.example.com/" a:localName="1" b:localName="2" />