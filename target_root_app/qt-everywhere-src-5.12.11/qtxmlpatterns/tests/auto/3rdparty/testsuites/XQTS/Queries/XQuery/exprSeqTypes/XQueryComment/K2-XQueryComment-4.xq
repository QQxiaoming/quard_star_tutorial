(:*******************************************************:)
(: Test: K2-XQueryComment-4                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Have a direct comment constructor as a last step. :)
(:*******************************************************:)
let $i := <e>
                                            <b/>
                                            <b/>
                                            <b/>
                                        </e>
                                        return $i/(: some : content (:some content:):)<!--content-->  