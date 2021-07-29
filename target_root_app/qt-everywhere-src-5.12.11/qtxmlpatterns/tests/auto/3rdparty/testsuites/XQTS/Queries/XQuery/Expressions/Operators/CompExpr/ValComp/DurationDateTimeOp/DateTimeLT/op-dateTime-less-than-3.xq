(:*******************************************************:)
(:Test: op-dateTime-less-than-3                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 17, 2005                                    :)
(:Purpose: Evaluates The "dateTime-less-than" function that  :)
(:return true and used together with fn:not (lt operator):)
(:*******************************************************:)
 
fn:not((xs:dateTime("2002-04-02T12:00:00Z") lt xs:dateTime("2003-04-02T12:00:00Z")))