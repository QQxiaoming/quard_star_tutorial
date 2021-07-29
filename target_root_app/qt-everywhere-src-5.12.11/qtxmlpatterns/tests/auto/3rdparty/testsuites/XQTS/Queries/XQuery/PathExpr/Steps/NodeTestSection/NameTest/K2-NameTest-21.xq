(:*******************************************************:)
(: Test: K2-NameTest-21                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A processing-instruction() with an invalid NCName in a string literal. :)
(:*******************************************************:)
empty(let $e := <a b ="content"><?b asd?><b/></a>
              return $e/processing-instruction("123ncname"))