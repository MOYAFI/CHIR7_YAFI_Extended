import { ToolDefinition } from './consolidated-tool-definitions.js';
import { commonSchemas } from './tool-definition-utils.js';

export const pcgToolDefinition: ToolDefinition = {
  name: 'manage_pcg',
  category: 'world',
  description: 'Control Procedural Content Generation (PCG) graphs, attributes, and execution. Create graphs, add nodes, manage attributes, and run generation.',
  inputSchema: {
    type: 'object',
    properties: {
      action: {
        type: 'string',
        enum: [
          'create_graph', 'add_node', 'connect_nodes', 'remove_node', 'get_graph_details',
          'set_attribute', 'get_attributes', 'add_attribute', 'remove_attribute',
          'generate', 'cleanup', 'set_seed', 'get_pcg_info', 'validate_graph'
        ],
        description: 'PCG action to perform'
      },
      assetPath: commonSchemas.assetPath,
      name: commonSchemas.name,
      path: commonSchemas.directoryPathForCreation,
      nodeType: { type: 'string', description: 'PCG node class (e.g., PCGStaticMeshSpawner, PCGPointFilter).' },
      nodeId: commonSchemas.nodeId,
      sourceNodeId: commonSchemas.sourceNodeId,
      targetNodeId: commonSchemas.targetNodeId,
      sourcePin: commonSchemas.pinName,
      targetPin: commonSchemas.pinName,
      attributeName: { type: 'string', description: 'Name of PCG attribute.' },
      attributeType: { type: 'string', enum: ['Float', 'Int32', 'Vector', 'String', 'Bool', 'Rotator', 'Transform'], description: 'Type of PCG attribute.' },
      value: { description: 'Value to set for attribute.' },
      seed: { type: 'number', description: 'Random seed for PCG generation.' },
      save: commonSchemas.save
    },
    required: ['action']
  },
  outputSchema: {
    type: 'object',
    properties: {
      ...commonSchemas.outputBase,
      assetPath: commonSchemas.assetPath,
      graphInfo: {
        type: 'object',
        properties: {
          nodeCount: { type: 'number', description: 'Number of nodes in graph.' },
          attributes: { type: 'array', items: { type: 'object' }, description: 'List of graph attributes.' },
          isGenerated: { type: 'boolean', description: 'Whether the graph has been generated.' }
        }
      },
      error: commonSchemas.stringProp
    }
  }
};
