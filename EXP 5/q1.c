#include<stdio.h>
struct node
{
    unsigned dist[20];
    unsigned from[20];
}rt[20];

int main()
{
    int costmat[20][20];
    int nodes,i,j,k,count=0;
    printf("Enter the number of nodes : ");
    scanf("%d",&nodes);//Enter the nodes
    int links=0;
    printf("Enter the number of links : ");
    scanf("%d",&links);
    printf("The edges and their weights line by line:\n");
    for(i=0;i<nodes;i++)
    {
        for(j=0;j<nodes;j++)
        {
            costmat[i][j]=999;
        }
    }
    
    for(i=0;i<links;i++)
    {
        
            int a,b,w;
            scanf("%d %d %d",&a,&b,&w);
            costmat[a][b]=w;
            costmat[b][a]=w;
            costmat[a][a]=0;
            costmat[b][b]=0;
        
    }
    
    

    for(i=0;i<nodes;i++)
    {
        for(j=0;j<nodes;j++)
        {
            costmat[i][i]=0;
            rt[i].dist[j]=costmat[i][j];//initialise the distance equal to cost matrix
            rt[i].from[j]=j;
        }
    }
    
    
        do
        {
            count=0;
            for(i=0;i<nodes;i++)//We choose arbitary vertex k and we calculate the direct distance from the node i to k using the cost matrix
            //and add the distance from k to node j
            for(j=0;j<nodes;j++)
            for(k=0;k<nodes;k++)
                if(rt[i].dist[j]>costmat[j][k]+rt[k].dist[i])
                {//We calculate the minimum distance
                    rt[i].dist[j]=rt[i].dist[k]+rt[k].dist[j];
                    rt[i].from[j]=k;
                    count++;
                }
        }while(count!=0);
        for(i=0;i<nodes;i++)
        {
            printf("\n\n For router %d\n",i);
            for(j=0;j<nodes;j++)
            {
                printf("\t\nnode %d via %d Distance %d ",j,rt[i].from[j],rt[i].dist[j]);
            }
        }
    printf("\n\n");
}
