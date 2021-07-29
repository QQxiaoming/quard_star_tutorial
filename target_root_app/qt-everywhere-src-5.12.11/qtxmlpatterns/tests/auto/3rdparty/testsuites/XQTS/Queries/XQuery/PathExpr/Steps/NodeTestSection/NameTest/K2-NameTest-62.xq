(:*******************************************************:)
(: Test: K2-NameTest-62                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply processing-instruction() to the result of an element constructor. :)
(:*******************************************************:)
<e>{for $PI in <e/>/processing-instruction() return ()}</e>