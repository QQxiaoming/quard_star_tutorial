(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-17                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that the right namespace binding is picked up. :)
(:*******************************************************:)
declare default element namespace "http://example.com/";
for $test as attribute(integer, xs:anyAtomicType) in (<e integer="1"/>/@integer)
return data($test)