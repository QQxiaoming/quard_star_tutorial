(:*******************************************************:)
(: Test: K2-NameTest-16                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A wild-card name test matching only elements. :)
(:*******************************************************:)
declare variable $e := <a b ="content"><?b asd?><b/></a>;
              $e/*