(:*******************************************************:)
(: Test: K2-NameTest-31                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A element name cannot be a string literal, inside element(). :)
(:*******************************************************:)
declare namespace a = "http://example.com/1";
                  declare namespace b = "http://example.com/2";
                            let $e := <e a:n1="content" b:n1="content">
                                        <a:n1/>
                                        <b:n1/>
                                        <?n1 ?>
                                        <n1/>
                                    </e>
                            return $e/a:*