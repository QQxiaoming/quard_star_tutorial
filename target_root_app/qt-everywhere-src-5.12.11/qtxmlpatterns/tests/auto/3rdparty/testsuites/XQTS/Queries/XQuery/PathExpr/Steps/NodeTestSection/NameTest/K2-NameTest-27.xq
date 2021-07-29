(:*******************************************************:)
(: Test: K2-NameTest-27                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A variable reference cannot be specified as PI name. :)
(:*******************************************************:)
let $name := "ncname"
                  return <e/>/processing-instruction($name))