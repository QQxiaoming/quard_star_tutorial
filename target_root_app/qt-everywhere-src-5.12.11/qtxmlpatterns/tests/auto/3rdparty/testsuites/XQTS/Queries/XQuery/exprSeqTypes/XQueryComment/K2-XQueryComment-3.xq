(:*******************************************************:)
(: Test: K2-XQueryComment-3                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have a computed comment constructor as a last step. :)
(:*******************************************************:)
let $i := <e>
                                            <b/>
                                            <b/>
                                            <b/>
                                        </e>
                                        return $i/b/comment(: some : content (:some content:):){"content"}  