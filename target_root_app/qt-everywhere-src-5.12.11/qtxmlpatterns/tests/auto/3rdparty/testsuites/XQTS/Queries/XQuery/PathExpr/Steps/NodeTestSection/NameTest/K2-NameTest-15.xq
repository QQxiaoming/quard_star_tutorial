(:*******************************************************:)
(: Test: K2-NameTest-15                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A name test matching many different kinds of nodes. :)
(:*******************************************************:)
declare variable $e := <a b ="content"><?b asd?><b/><c b="content"/></a>;
                  $e/b