(:*******************************************************:)
(:Test: op-dateTime-greater-than-3                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 15, 2005                                    :)
(:Purpose: Evaluates The "dateTime-greater-than" function that :)
(:return true and used together with fn:not (gt operator):)
(:*******************************************************:)
 
fn:not((xs:dateTime("2004-04-02T12:00:00Z") gt xs:dateTime("2003-04-02T12:00:00Z")))