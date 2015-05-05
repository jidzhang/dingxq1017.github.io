AcDbObjectId CreatePolyline(AcGePoint2dArray points, double width)//创建polyline
{
        int numVertices = points.length();
        AcDbPolyline *pPoly = new AcDbPolyline(numVertices);
        for (int i = 0; i < numVertices; i++)
        {
                pPoly->addVertexAt(i, points.at(i), 1, width, width);
        }
        AcDbObjectId polyId;
        polyId = PostToModelSpace(pPoly);
        return polyId;
}

void CHAPtest2()
{
        AcDbCircle *cl;                double radiu;                
        AcGePoint3d center;              
        AcGePoint2d pt[3];               
        AcGePoint2dArray points1;

        AcDbPolyline *pPoly;                
        AcDbEntity * pEnt;                
        ads_name entName;                
        long len=0;                
        AcDbObjectId entId;                
        AcDbObjectIdArray objIds;

        acedSSGet(NULL,NULL,NULL,NULL,entName);              
        int rt=acedSSLength(entName,&len);                         
        if (rt == RTNORM)                
        {                      
                long length;                      
                acedSSLength(entName, &length); // 获得选择集中的对象个数                        
                for (int i = 0; i < length; i++)                       
                {                               
                        ads_name ent;                               
                        acedSSName(entName, i, ent);                                            
                        acdbGetObjectId(entId, ent);                              
                        objIds.append(entId);                       
                }               
        }             
        else         
        {                        
                MessageBox(NULL,"请选择一个实体","注意",MB_OK);                        
                return ;                
        }                        
        acedSSFree(entName);                               
        for (int j1=0;j1<objIds.length();j1++)                
        {                                       
                acdbOpenObject(pEnt, objIds[j1],AcDb::kForWrite);                        
                if (pEnt->isKindOf(AcDbCircle::desc()))                        
                {                                                                
                        cl=AcDbCircle::cast(pEnt);                                
                        center=cl->center();                                
                        radiu=cl->radius();                        
                }
        }                 
        //建立一条有3个顶点（p1、p2、p1）的多段线，分别设这两段的凸度值为1（CreatePolyline）                
        pt[1].x=pt[2].x=pt[3].x=center.x;               
        pt[1].y=center.y+radiu;               
        pt[2].y=center.y-radiu;                
        pt[3].y=center.y+radiu;                
        for(int i=1;i<=3;i++)               
        {                
                points1.append(pt);               
        }

        CreatePolyline(points1,0);                
        cl->erase();                
        pEnt->close();        
}
