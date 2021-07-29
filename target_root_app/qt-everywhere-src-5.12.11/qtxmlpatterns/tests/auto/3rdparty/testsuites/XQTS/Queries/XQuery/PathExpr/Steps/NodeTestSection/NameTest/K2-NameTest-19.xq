(:*******************************************************:)
(: Test: K2-NameTest-19                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A name test matching only processing-instructions. :)
(:*******************************************************:)
declare variable $e := <a b ="content"><?b asd?><b/></a>;
$e/processing-instruction(b)